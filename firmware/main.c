/***************************************************************************
 *   Copyright (C) 2012 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   www.legalethurlant.fr.st                                              *
 *                                                                         *
 *   This file is part of SolexTronic                                      *
 *                                                                         *
 *   SolexTronic is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 /**
 * \file main.c
 * \brief Main file of the project
 * \author Thibault Bouttevin
 * \date October 2012
 *
 * This file includes main function and main loop
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "platform.h"
#include "command.h"
#include "helper.h"
#include "frontpanel.h"
#include "chrono.h"

volatile eeprom_data_t    eData;
volatile Current_Data_t   gState;
volatile intState_t       intState;
volatile u16 toto = 0;


#define CYL (50)	// 50 cm3 = 5e-5m3

u16 ComputeK(u16 patm)
{
	/* K = MMAir.CYL.Patm / AFR.R
	* MMAir = 28,965338 g·mol-1
	* CYL = 50cm3 => *1e-6m3
	* pAtm in hPa => *100Pa
	* AFR no unit
	* R = 8,3144621 J·mol-1·K-1
	* Patm, AFR are semi-static meaning it won't change after boot => function can be called once in init (only need a valid MAP measurement) 
	* K = (MMAir/R).CYL/1e6.PAtm*100/(AFR10/10)
	* K = 3.48373e-3.CYL.Patm/AFR10
	* K = 3.484.CYL.Patm/AFR10 with output in mg
	*/
	return  (u16)(3484L * CYL * (u32)patm / (1000 * eData.targetAfr)); 	
}

u8 ComputeIgnition(u8 overheat)
{
    //TODO manage load based on throttle/pressure/whatever. Force to 50% for now
    gState.load = 50; 
    // compute advance from table
    gState.advance = Interp2D(&(eData.ignTable[0][0]), gState.rpm, gState.load);

    // TODO set adjustement for acceleration
    
    // set limitation for overheating
    if(overheat)
    {
        gState.advance -= eData.ignOverheat;
    }
    
    // Add runtime offset
    gState.advance += gState.ignOffset;

    // commit advance for next cycle
    SetIgnitionTiming(AUTO, gState.advance);

    return OK;
}   

u8 ComputeInjection(u8 overheat)
{
    //TODO manage load based on throttle/pressure/whatever. Force to 50% for now
    gState.load = 50; 
    // compute VE from table
    gState.injVE = Interp2D(&(eData.injTable[0][0]), gState.rpm, gState.load);
	
	// apply formula, K has been computed before (semi-static). QFuel is in mg
	gState.injQFuel = gState.injK * gState.injVE / (gState.IAT + 273);

	// add various enrichments
	u16 enrich = 0;
    if(overheat)
    { 
        enrich += eData.injOverheat;
    }
	if(intState.afterStartPeriod)
	{
		enrich += gState.injAfterStartEnrich;
		intState.afterStartPeriod--;
	}else{
		gState.injAfterStartEnrich = 0;
	}
	
	enrich += gState.injWarmupEnrich;
    
	// TODO set adjustement for acceleration
    
    // Add runtime offset
    enrich += gState.injOffset;
    
	// turn into injector pulse width
	gState.injPulseWidth = (gState.injQFuel * (100 + enrich)) / (100 * eData.injRate) + eData.injOpen;

    // commit advance for next cycle
    SetInjectionTiming(AUTO, gState.injPulseWidth);

    return OK;
}

u8 MainFsm(void)
{
    switch(gState.engineState)
    {
        case M_STOP: // engine stopped : no injection/ignition (except test)
            SetInjectionTiming(FORCEOFF, 0);
            SetInjectionTiming(FORCEOFF, 0);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            PIN_OFF(HV_PIN, HV_POLARITY);
            if(!PIN_READ(CRANKING_PIN))
            {
				// latch CLT to compute afterstart enrich
				gState.injAfterStartEnrich = Interp1D(eData.injAfterStartTbl, gState.CLT);
				// compute K
				gState.injK = ComputeK(gState.MAP);
				gState.advance = eData.ignStarter;
				gState.injPulseWidth = ((u32)eData.injStarter * (100 + gState.injAfterStartEnrich) / 100) + eData.injOpen;
				SetIgnitionTiming(AUTO, gState.advance);
				SetInjectionTiming(AUTO, gState.injPulseWidth);
                gState.engineState = M_CRANKING;
            }
        break;

        case M_TEST_INJ: // injector test mode
            PIN_ON(PUMP_PIN, eData.pumpPolarity);
            PIN_OFF(HV_PIN, HV_POLARITY);
        break;
        
        case M_TEST_IGN: // ignition test mode
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            PIN_ON(HV_PIN, HV_POLARITY);
        break;

        case M_CRANKING: // someone is pushing ! force inj/ign to crancking values until we reach 1000rpm
            PIN_ON(PUMP_PIN, eData.pumpPolarity);
            PIN_ON(HV_PIN, HV_POLARITY);
            if(PIN_READ(CRANKING_PIN))
            {
                gState.engineState = M_STOP;
            }
			if(gState.rpm > 1000)
			{
				intState.afterStartPeriod = eData.injAfterStartDur;
				gState.engineState = M_RUNNING;
			}
        break;

        case M_RUNNING: // normal operation: apply formula, recomputed each new cycle
            if(intState.newCycle) 
            {
                intState.newCycle = 0;
                u8 overheat = eData.maxTemp ? (gState.CLT > eData.maxTemp) : 0;
                ComputeInjection(overheat);
                ComputeIgnition(overheat);
            }
        break;

        case M_ERROR: // that's bad : stop everything
            SetInjectionTiming(FORCEOFF, 0);
            SetInjectionTiming(FORCEOFF, 0);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            PIN_OFF(HV_PIN, HV_POLARITY);
        break;

        case M_STALLED: // wait for cranking
            SetInjectionTiming(FORCEOFF, 0);
            SetInjectionTiming(FORCEOFF, 0);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            PIN_OFF(HV_PIN, HV_POLARITY);
            if(!PIN_READ(CRANKING_PIN))
            {
                gState.engineState = M_CRANKING;
            }
        break;

        default:
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            PIN_OFF(HV_PIN, HV_POLARITY);
            ASSERT(0);
    } // switch

    return gState.engineState;
}

int main(void)
{
    // init var
    memset((char *)&gState, 0, sizeof(gState));
    memset((char *)&intState, 0, sizeof(intState));
	
    InitIOs();
    InitUart();
    InitTimer();
    ADCInit();
#ifdef SIM
    InitEeprom(1); // force init
#else
    InitEeprom(0);
    FPInit(0);
#endif
    ChronoInit();
    gState.engineState = M_STOP; 

	sei();
    StartTimer(TIMER_100MS);
    printstr("x");
    USART_RX_EN;
    eData.timerLed = 100;
 
	while(1)
	{
        updateEeprom();

        if(EndTimer(TIMER_100MS, eData.timerLed))
        {
            StartTimer(TIMER_100MS);
            PIN_TOGGLE(LED_PIN);
#if(!SIM)
            FPRun();
#endif
        }
        
        if(intState.rReady)
		{
            intState.rReady = 0;
            ProcessCommand();
		}

        // Process ADC Samples
        if(intState.adcDone) 
        {
            ADCProcessing();
        }
        
        // Compute ignition and injection
        MainFsm();
	}

	return(0);
}

