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

#define LED_DDR		DDRB
#define LED_PORT	PORTB
#define LED			PB5
#define LED_PIN		PINB

#define MAXSTR 	10

volatile unsigned char rReady = 0;

Flags_t          Flags;
eeprom_data_t    eData;
Current_Data_t   CurrentValues;


int main(void)
{
	LED_DDR |= _BV(LED);
	
    InitIOs();
    InitUart();
    InitTimer();
    eData.timerLed = 100;
    eData.wheelSize = 182;

	sei();
    StartTimer(TIMER_100MS);
    printstr("demarrage");
    USART_RX_EN;

	while(1)
	{
        if(EndTimer(TIMER_100MS, eData.timerLed))
        {
            LED_PIN |= _BV(LED);
            StartTimer(TIMER_100MS);
        }
        
        if(rReady)
		{
			if(rReady == 1)
			{
                rReady = 0;
                ProcessCommand();
                USART_RX_EN;
			}
		}

        // Process ADC Samples
        ADCProcessing();
	}

	return(0);
}

