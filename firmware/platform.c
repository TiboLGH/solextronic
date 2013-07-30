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
#include <avr/eeprom.h>
#include "common.h"
#include "platform.h"

#define BAUD 	        57600
#define HTINPUT 	    (PIND & (1 << PD6))

uint16_t EEMEM magic = 0xCAFE;
eeprom_data_t EEMEM eeprom;

extern Flags_t          Flags;
extern eeprom_data_t    eData;
extern Current_Data_t   CurrentValues;

extern uint8_t bufTx[];
extern uint8_t bufRx[];
extern uint8_t indexRx;
extern uint8_t indexTxRead;
extern uint8_t indexTxWrite;
extern uint8_t rReady;

static volatile uint16_t adcValues[5];
const uint8_t adcIndex[] = {0, 1, 2, 3, 6, 255};
static volatile uint8_t adcState;
static volatile uint8_t adcTrigger;
static uint32_t	timerTable[TIMER_QTY];
static volatile uint32_t masterClk;
static volatile uint8_t count10ms;
static TimeStamp_t     prevTs, newTs; 



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
 * \fn void InitIOs(void)
 * \brief Init GPIO directions and mode
 * \param none
 * \return none
 */
void InitIOs(void)
{
    /* HTINPUT on PD6 : input, pull-up enable
       Dead high on PD2/INT0 : input, pull-up, interrupt on rising edge
       Wheel synchro on PD3/INT1 : input, pull-up, interrupt on rising edge */
    DDRD  = 0b10110010;
    PORTD = 0b10110010;

    EICRA = (1 << ISC11) | (1 << ISC10) | (1 << ISC01) | (1 << ISC00);
    EIMSK = (1 << INT1) | (1 << INT0);
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
    /* read 2 first bytes to check magic : 
     * allow to see if eeprom is void */
    if(0xCAFE != eeprom_read_word(&magic))
    {
        /* initialise eeprom */
    } 

    /* read all data from EEPROM to cache */
    eeprom_read_block((void*)&eData, &eeprom, sizeof(eeprom_data_t)); 
    return;
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
    static uint8_t index = 0;
    uint8_t *pData = &eData;
    if(eeprom_is_ready())
    {
        eeprom_update_byte((uint8_t*)(&eeprom + index), *(pData + index));
        index = (index == sizeof(eeprom_data_t))? 0: index++;
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
    masterClk++;
    count10ms++;
    if(count10ms >= 10)
    {
        count10ms = 0;
        // start ADC acquisition
        ADMUX = (1 << REFS0) | ((adcIndex[adcState]) & 0x0F);
        ADCSRA |= (1 << ADSC);
        adcTrigger = True;

        // run HV loop
        if(eData.HVstep)
        {
            if(HTINPUT == 0)
            {
                CurrentValues.HVvalue += eData.HVstep;
            }else{
                CurrentValues.HVvalue -= eData.HVstep;
            }
            if(CurrentValues.HVvalue > 100) CurrentValues.HVvalue = 100;
            else if((signed)CurrentValues.HVvalue < 0) CurrentValues.HVvalue = 0;
            WritePWMValue(CurrentValues.HVvalue);
        } 

    }
}

/**
 * \fn ISR(TIMER1_OVF_vect)
 * \brief Interrupt on overflow of Timer1 : engine is stalled
 *
 * \param none
 * \return none
 */
ISR(TIMER1_OVF_vect)
{
    CurrentValues.state = STALLED;
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
    // Enable the TIMER1, set normal mode, internal clock with 1/64 prescaler.
    // With 16MHz XTAL, tick is 4us
    // Overflow is 262 ms = 229 tr/min => considered as stall
    TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0) | (0 << WGM11) | (0 << WGM10);
    TCCR1B = (0 << ICNC1 ) | (1 << ICES1 ) | (0 << WGM13 ) | (0 << WGM12 ) | (0 << CS12 ) | (1 << CS11 ) | (1 << CS10);
    TCNT1H = 0;
    TCNT1L = 0;
    TIMSK1 = (0 << ICIE1) | (0 << OCIE1B) | (0 << OCIE1A) | (1 << TOIE1);
    
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
 * \brief Fired on conversion end. Set next conversion parameter and restart conversion
 *
 * \param none
 * \return none
*/
ISR(ADC_vect)
{
    // save results
    adcValues[adcState] = ADC;
    // Next channel
    adcState++;
    // start ADC acquisition
    if(adcState < ADC_IDLE)
    {
        ADMUX = (1 << REFS0) | ((adcIndex[adcState]) & 0x0F);
        ADCSRA |= (1 << ADSC);
    }
}

/**
 * \fn void ADCInit(void)
 * \brief Init ADC inputs
 *
 * \param none
 * \return none
*/
void ADCInit(void)
{
    ADCSRA = (1 << ADEN)  |
             (0 << ADATE) |
             (1 << ADIE) | 
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
    DIDR0 = 0x0F; // set ADC0..3 pin to ADC inputs
    adcTrigger = False; 
    adcState = ADC_BATTERY;
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
    static uint8_t adcProcessed = 0;

    if(adcTrigger)
    {
        // Conversion and filtering
        switch(adcIndex[adcProcessed])
        {
            case ADC_BATTERY:
                if(adcState > ADC_BATTERY) // Battery voltage
                {
                    CurrentValues.battery = adcValues[ADC_BATTERY]; //3/2 * eData.ratio[ADC_BATTERY] * adcValues[ADC_BATTERY] / 100;     
                }
                break;

            case ADC_TEMP1:
                if(adcState > ADC_TEMP1)
                {
                    CurrentValues.temp1 = eData.ratio[ADC_TEMP1] * adcValues[ADC_TEMP1] / 100;     
                }
                break;
            
            case ADC_TEMP2:
                if(adcState > ADC_TEMP2)
                {
                    CurrentValues.temp2 = eData.ratio[ADC_TEMP2] * adcValues[ADC_TEMP2] / 100;     
                }
                break;

            case ADC_THROTTLE:
                    if(adcState > ADC_THROTTLE)
                {
                    CurrentValues.throttle = (eData.ratio[ADC_THROTTLE] * adcValues[ADC_THROTTLE]) / 256;     
                }
                break;
            
            case ADC_PRESSURE:
                if(adcState > ADC_PRESSURE)
                {
                    CurrentValues.pressure = eData.ratio[ADC_PRESSURE] * adcValues[ADC_PRESSURE] / 100;     
                }
                break;

            default:
                //ASSERT(0);
                adcState = ADC_BATTERY;
        }
        adcProcessed++;
    }
    else
    {
       adcProcessed = 0;
    } 
}

/**
 * \fn void InitPWM(void)
 * \brief Initialisation of the PWM block (with Timer0)
 *
 * \return none 
*/
void InitPWM(void)
{
    // Set PD5/OC0B as output
    DDRD |= (1 << DDD5);
    // Fast PWM, prescaler 1 (~60kHz), output on OC0B
    TCCR0A = (0 << COM0A1) | (0 << COM0A0) |
             (1 << COM0B1) | (0 << COM0B0) |
             (1 << WGM01)  | (1 << WGM00);
    TCCR0B = (0 << FOC0A)  | (0 << FOC0B) |
             (1 << WGM02)  | (0	<< CS02)  |
             (0 << CS01)   | (1 << CS00); 

    OCR0B = 0; // off state
    CurrentValues.HVvalue = 0;
}

/**
 * \fn void WritePWMValue(uint8_t value)
 * \brief write value to PWM
 *
 * \param uint8_t value : value to set in %
 * \return none
*/
void WritePWMValue(uint8_t value)
{
    OCR0B = (uint8_t)(value * 255 / 100); 
}

/**
 * \fn ISR(INT0_vect)
 * \brief Fired on INT0 rising/falling edge at dead high point to measure RPM, start ignition/injection cycle
 *
 * \param none
 * \return none
*/
ISR(INT0_vect)
{
    // Save timer value
    uint16_t period; 
    period = TCNT1;
    // clear timer for next period
    TCNT1 = 0;
    // compute RPM : tick is 4us
    CurrentValues.RPM = 60 * (250000 / period);
    CurrentValues.state = RUNNING;
}

/**
 * \fn ISR(INT1_vect)
 * \brief Fired on INT1 rising/falling edge of wheel captor
 *
 * \param none
 * \return none
*/
ISR(INT1_vect)
{
    uint16_t period; // unit is 4us
    // save current timestamp
    newTs.clk  = masterClk;
    newTs.tick = TCNT2;

    // Compute period
    period = (newTs.clk - prevTs.clk) * 250;
    period += (signed)(newTs.tick - prevTs.tick);

    prevTs.clk  = newTs.clk;
    prevTs.tick = newTs.tick;

    // Compute speed in 1/10 of km/h
    //wh/period = cm/4us
    CurrentValues.speed = ((uint32_t)eData.wheelSize * 25 * 3600) / period;
}
