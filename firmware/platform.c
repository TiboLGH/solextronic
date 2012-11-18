/***************************************************************************
 *   Copyright (C) 2012 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   www.legalethurlant.fr.st                                              *
 *                                                                         *
 *   This file is part of SolexTronic                                      *
 *                                                                         *
 *   Solextronic is free software; you can redistribute it and/or modify   *
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
 * \file platform.c
 * \brief Manage all low-level and hardware interfaces.
 * \author Thibault Bouttevin
 * \version 0.1
 * \date February 2012
 *
 * This file implements low-level services and hardware interfaces including
 * USART emission/reception, EEPROM management, software timers, ADC, PWM and SPI bus.
 * It is targetted for ATMega328p on Arduino Nano.
 *
 */

#include <assert.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "common.h"
#include "platform.h"

#define BAUD 	57600

extern Flags_t          Flags;
extern eeprom_data_t    eData;
extern Current_Data_t   CurrentValues;

extern uint8_t bufTx[];
extern uint8_t bufRx[];
extern uint8_t indexRx;
extern uint8_t indexTxRead;
extern uint8_t indexTxWrite;
extern uint8_t rReady;

const uint8_t ECHO_CMD[] = "Le Galet Hurlant";


static uint32_t	timerTable[TIMER_QTY];
void (*masterClkCB[])(void) = {ADCProcessing};
const uint8_t nbCB = 1;
static uint32_t masterClk;

const uint8_t default_parameters [] = {
   100, 100, 100, 100, 100,             /* ratio for ADC and DAC */
   0x03, 0xE8,					      /* Vset : 10v */
   0x00, 0x64,					      /* Iset : 1 A */
};


/**
 * \fn void InitUsart(void)
 * \brief AVR usart
 *
 * \param none
 * \return none
*/
void InitUart(void)
{
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= (1 << U2X0);
#else
	UCSR0A &= ~(1 << U2X0);
#endif
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	UCSR0B |= (1 << RXCIE0);
	indexTxWrite = 0;
	indexTxRead  = 0;
	indexRx      = 0;
	return;
}

void putchr(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

void printstr (unsigned char *string){
	while(*string){
		putchr(*string++);
	}
	putchr('\r');
	putchr('\n');
}

ISR(USART_RX_vect)
{
	char c;
	c = UDR0;
	if(bit_is_clear(UCSR0A, FE0))
	{
		if(c != '\r')
		{
            bufRx[indexRx++] = c;
		}else{
            bufRx[indexRx++] = c; // let the CR
            bufRx[indexRx++] = 0; 
			rReady = 1;
			USART_RX_DIS;
		}
	}
}

ISR(USART_TX_vect)
{
    if(indexTxWrite != indexTxRead)
    {
        UDR0 = bufTx[indexTxRead++];
    }else{
        USART_TX_DIS; 
    }
}


/**
 * \fn void InitEeprom(void)
 * \brief Init EEPROM management at PIC start up
 *
 * \param none
 * \return none
 */
void InitEeprom(void)
{
   uint8_t i;
   /* read all data in EEPROM */
   for(i = 0; i < sizeof(eeprom_data_t); i ++)
   {
       ((uint8_t *) &eData)[i] = readEeprom(i);
   }
   
   return;
}

/**
 * \fn void resetEeprom(void)
 * \brief set default parameters in EEPROM
 *
 * \param none
 * \return none
 */
void resetEeprom(void)
{
   uint8_t i;

   /* reset all */
   for(i = 0; i < sizeof(eeprom_data_t); i ++)
   {
     ((uint8_t *) &eData)[i] = default_parameters[i];
   }	
	
   return;
}

/**
 * \fn uint8_t readEeprom(uint8_t address)
 * \brief read one byte from EEPROM
 *
 * \param address EEPROM address
 * \return none
 */
uint8_t readEeprom(uint8_t address)
{
   return 0;    // recover data from eeprom byte to byte
}

/**
 * \fn void writeEeprom(uint8_t address, uint8_t value)
 * \brief write one byte to EEPROM
 *
 * \param address EEPROM address
 * \param value value to write
 * \return none
 */
void writeEeprom(uint8_t address, uint8_t value)
{
   return ;
}


/**
 * \fn void updateEeprom(void)
 * \brief Update EEPROM
 *
 * \param none
 * \return none
 */
void updateEeprom(void)
{
    static uint8_t 	state = 0;
    static uint8_t    index = 0;

    if(state == 0)
    {
        /* in this case, one byte is different, so EEPROM must be updated */
        if(((uint8_t *) &eData) [index] != readEeprom(index))
        {
            writeEeprom(index,((uint8_t *) &eData)[index]);
            state = 1;
        }else        {
            index ++;	
            if(index == sizeof(eeprom_data_t))
            {
                index = 0;
            }
        }
    }
    return;
}

/**
 * \fn ISR(TIMER2_COMPA_vect)
 * \brief Interrupt on compare of Timer2 : 1ms period
 *
 * \param none
 * \return none
 */

ISR(TIMER2_COMPA_vect)
{
    uint8_t i;
    masterClk++;
    // attached call backs 
    for(i = 0; i < nbCB; i++) (*masterClkCB[i])();
}

/**
 * \fn void InitTimer(void)
 * \brief Init software timers
 *
 * \param none
 * \return none
 */
void InitTimer(void)
{
    // Enable the TIMER2, set CTC mode, internal clock with 1/64 prescaler.
    // With 16MHz XTAL, tick is 4us
    // To get 1ms period, set compare to 250

    TCCR2A = 0x02;
    TCCR2B = 0x04;
    OCR2A  = 250;
    TIMSK2 = (0 << OCIE2B) | (1 << OCIE2A) | (0 << TOIE2);

    return;
}


/**
 * \fn void StartTimer(uint8_t timerHandle)

 * \brief start a software timer
 *
 * \param timerHandle handle on the timer to start
 * \return none
 *
 * To start timer, internal counter is set to master clock value
 */
void StartTimer(uint8_t timerHandle)
{
   timerTable[timerHandle] = masterClk;
   return;
}


/**
 * \fn unsigned long GetTimer_ms(const uint8_t timerHandle)

 * \brief return number of milliseconds of timer defined by timerHandle
 *
 * \param timerHandle handle on the timer to get
 * \return number of milliseconds of timer defined by timerHandle
 *
 */
unsigned long GetTimer(const uint8_t timerHandle)
{
   return masterClk - timerTable[timerHandle];
}


/**
 * \fn uint8_t EndTimer(const uint8_t timerHandle, const unsigned long duration)

 * \brief return if duration is over on timer defined by timerHandle

 *
 * \param timerHandle handle on the timer to get
 * \param duration wait time in msec
 * \return 0 if duration is not issued, 1 if issued
 *
 */
uint8_t EndTimer(const uint8_t timerHandle, const unsigned long duration)
{
   if(masterClk - timerTable[timerHandle] > duration)
      return 1;
   else
      return 0;
}

/**
 * \fn ISR(ADC_vect)
 * \brief End on conversion interrupt
 *
 * \param none
 * \return none
*/
ISR(ADC_vect)
{
}

/**
 * \fn void InitADC(void)
 * \brief Init ADC inputs
 *
 * \param none
 * \return none
*/
void InitADC(void)
{
}

/**
 * \fn void ADCProcessing(void)
 * \brief run ADC state machine
 *
 * \param none
 * \return none
*/
void ADCProcessing(void)
{
}

/**
 * \fn void ADCCompute(void)
 * \brief convert and filter ADC values
 *
 * \param none
 * \return none
*/
void ADCCompute(void)
{
}

/**
 * \fn void InitPWM(void)
 * \brief Initialissation of the PWM
 *
 * \return none 
*/
void InitPWM(void)
{
}

/**
 * \fn void writePWMValue(uint8_t value)
 * \brief write value to PWM
 *
 * \param uint8_t value : value to set in %
 * \return none
*/
void writePWMValue(uint8_t value)
{
}

