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
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "platform.h"
#include "command.h"
#include "helper.h"

volatile eeprom_data_t    eData;
volatile Current_Data_t   gState;
volatile intState_t       intState;
volatile u16 toto = 0;


u8 ComputeIgnition(void)
{
    DEBUG = 44;
    //TODO manage load based on throttle/pressure/whatever. Force to 50% for now
    gState.load = 50; 
    // compute advance from table
    gState.advance = Interp2D(&(eData.igniTable[0][0]), gState.rpm, gState.load);

    // TODO set adjustement for acceleration
    
    // TODO set limitation for overheating

    // commit advance for next cycle
    SetIgnitionTiming(AUTO, gState.advance);
    DEBUG = 0;

    return OK;
}    

int main(void)
{
	LED_DDR |= _BV(5);
	LED_DDR |= _BV(1);
	LED_DDR |= _BV(2);
	
    InitIOs();
    InitUart();
    InitTimer();
    ADCInit();
    InitEeprom(1);

	sei();
    StartTimer(TIMER_100MS);
    printstr("x");
    USART_RX_EN;
    eData.timerLed = 50;
    DEBUG = 0;
 
	while(1)
	{
        toto = TCNT1;
        toto = OCR1A;
        toto = OCR1B;
        if(EndTimer(TIMER_100MS, eData.timerLed))
        {
            C_FLIPBIT(LED_PIN);
            StartTimer(TIMER_100MS);
            ADCProcessing();
        }
        
        if(intState.rReady)
		{
            intState.rReady = 0;
            ProcessCommand();
		}

       
        // Process ADC Samples
        //ADCProcessing();

        // Compute ignition and injection
        if(intState.newCycle) 
        {
            intState.newCycle = 0;
            ComputeIgnition();
        }
	}

	return(0);
}

