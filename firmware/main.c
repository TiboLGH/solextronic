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
    // compute injection delay from table
    gState.advance = Interp2D(&(eData.ignTable[0][0]), gState.rpm, gState.load);

    // TODO set adjustement for acceleration
    
    // set limitation for overheating
    if(overheat)
    {
        gState.injPulseWidth *= (100 + eData.injOverheat) / 100;
    }
    
    // Add runtime offset
    gState.injPulseWidth += gState.injOffset;
    
    // Injector open time
    gState.injPulseWidth += eData.injOpen;

    // commit advance for next cycle
    SetInjectionTiming(AUTO, gState.injPulseWidth);

    return OK;
}

u8 MainFsm(void)
{
    switch(gState.engineState)
    {
        case M_STOP: // engine stopped : no injection/ignition (except test)
            FPSetLed(VIOLET);
            SetInjectionTiming(FORCEOFF, 0);
            SetInjectionTiming(FORCEOFF, 0);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            if(!C_CHECKBIT(CRANKING_PIN))
            {
                gState.engineState = M_CRANKING;
            }
        break;

        case M_TEST_INJ: // injector test mode
            FPSetLed(TEAL);
            PIN_ON(PUMP_PIN, eData.pumpPolarity);
        break;
        
        case M_TEST_IGN: // ignition test mode
            FPSetLed(TEAL);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
        break;

        case M_CRANKING: // someone is pushing ! force inj/ign to crancking values
            FPSetLed(TEAL);
            gState.advance = eData.starterAdv;
            gState.injPulseWidth = eData.starterInj + eData.injOpen;
            SetIgnitionTiming(AUTO, gState.advance);
            SetInjectionTiming(AUTO, gState.injPulseWidth);
            PIN_ON(PUMP_PIN, eData.pumpPolarity);
            if(C_CHECKBIT(CRANKING_PIN))
            {
                gState.engineState = M_RUNNING;
            }
        break;

        case M_RUNNING: // normal operation: apply formula
            if(intState.newCycle) 
            {
                intState.newCycle = 0;
                FPSetLed(GREEN);
                u8 overheat = eData.maxTemp ? (gState.CLT > eData.maxTemp) : 0;
                ComputeInjection(overheat);
                ComputeIgnition(overheat);
            }
        break;

        case M_ERROR: // that's bad : stop everything
            FPSetLed(RED);
            SetInjectionTiming(FORCEOFF, 0);
            SetInjectionTiming(FORCEOFF, 0);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
        break;

        case M_STALLED: // wait for cranking
            FPSetLed(BLUE);
            SetInjectionTiming(FORCEOFF, 0);
            SetInjectionTiming(FORCEOFF, 0);
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            if(!C_CHECKBIT(CRANKING_PIN))
            {
                gState.engineState = M_CRANKING;
            }
        break;

        default:
            PIN_OFF(PUMP_PIN, eData.pumpPolarity);
            ASSERT(0);
    } // switch

    return gState.engineState;
}

int main(void)
{
	LED_DDR |= _BV(5);
	LED_DDR |= _BV(1);
	LED_DDR |= _BV(2);

    // init var
    memset((char *)&gState, 0, sizeof(gState));
    memset((char *)&intState, 0, sizeof(intState));
	
    InitIOs();
    InitUart();
    InitTimer();
    ADCInit();
    InitEeprom(1);
    FPInit(0);
    ChronoInit();
    gState.engineState = M_STOP; 

	sei();
    StartTimer(TIMER_100MS);
    printstr("x");
    USART_RX_EN;
    eData.timerLed = 100;
 
	while(1)
	{
        toto = TCNT1;
        toto = OCR1A;
        toto = OCR1B;


        if(EndTimer(TIMER_100MS, eData.timerLed))
        {
            StartTimer(TIMER_100MS);
            C_FLIPBIT(LED_PIN);
            FPRun();
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

