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

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "common.h"
#include "platform.h"

#define BAUD 	        57600
#define HTINPUT 	    (PIND & (1 << PD6))

u16 EEMEM magic = 0xCAFE;
eeprom_data_t EEMEM eeprom;
const PROGMEM eeprom_data_t eeInit = {
    .ratio          = {100, 100, 100, 100, 100},
    .timerLed       = 20,
    .HVstep         = 5,
    .HVmanual       = 50,
    .wheelSize      = 182,
    .PMHOffset      = 0,
    .maxRPM         = 0,
    .maxTemp        = 150,
    .minBat         = 110,
    .igniDuration   = 1000,
    .starterAdv     = 5,
    .igniOverheat   = 2,
    .noSparkAtDec   = 0,
    .injOpen        = 500,
    .injRate        = 500, //?
    .injAdv         = 140, //?
    .starterInj     = 1000,
    .injOverheat    = 3,
    .injFullOpen    = 3000,
    .noInjAtDec     = 0,
    .injStart       = 500,
    .holdPWM        = 50,
    .igniPolarity   = 0,
    .injPolarity    = 0,
    .pmhPolarity    = 0,
    .pumpPolarity   = 0,
    //.injTable[12][12];  /* let it to 0 */
    //.igniTable[12][12];
};

extern eeprom_data_t    eData;
extern Current_Data_t   gState;

extern u8 *bufTx;
extern u8 bufRx[];
extern u8 indexRx;
extern u8 rReady;
extern u16 txCur;
extern u16 txCount;
extern u8 isTx;

static volatile u16 adcValues[5];
const u8 adcIndex[] = {7, 1, 2, 3, 6, 255};
static volatile u8 adcState;
static volatile u8 adcTrigger;
static u32	timerTable[TIMER_QTY];
static volatile u32 masterClk;
static volatile u8 count10ms;
static TimeStamp_t     prevTs, newTs;
static volatile wvf_t curInjTiming = {.state = OFF, .start = 0, .stop = 0};
static volatile wvf_t nextInjTiming = {.state = OFF, .start = 0, .stop = 0};
static volatile wvf_t curIgnTiming = {.state = OFF, .start = 0, .stop = 0};
static volatile wvf_t nextIgnTiming = {.state = OFF, .start = 0, .stop = 0};
static volatile u16 RPMperiod; 


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
	indexRx      = 0;
	return;
}

void putchr(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

void printstr (char *string){
	while(*string){
		putchr(*string++);
	}
}


void __assert__(char *expr, char *filename, int linenumber)
{
    cli();
	printstr("FATAL\n");
	printstr(expr);
    printstr("\n");
	printstr(filename);
    printstr(" line ");
    char str[8];
    itoa(linenumber, (char *)str, 10);
	printstr(str);
    abort();
}

ISR(USART_RX_vect)
{
	if(bit_is_clear(UCSR0A, FE0))
	{
        bufRx[indexRx++] = UDR0;
        rReady = 1;
	}
}

ISR(USART_TX_vect)
{
    if(txCur != txCount)
    {
        UDR0 = bufTx[txCur];
        txCur++;
    }else{
        isTx = False;
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
 * \param force : if 1, force reinit EEPROM content
 * \return none
 */
void InitEeprom(u8 force)
{
    /* read 2 first bytes to check magic : 
     * allow to see if eeprom is void */
    if(force || (0xCAFE != eeprom_read_word(&magic)))
    {
        /* initialise eeprom */
        eeprom_write_word(&magic, 0xCAFE);
        memcpy_P((void *)&eData, (PGM_VOID_P)&eeInit, sizeof(eeprom_data_t));
    }else{
        /* read all data from EEPROM to cache */
        eeprom_read_block((void*)&eData, &eeprom, sizeof(eeprom_data_t)); 
    } 
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
    static u8 index = 0;
    u8 *pData = (u8 *)&eData;
    if(eeprom_is_ready())
    {
        eeprom_update_byte((u8*)(&eeprom + index), *(pData + index));
        index++;
        if(index > sizeof(eeprom_data_t)) index = 0;
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
        /*adcState = ADC_BATTERY;
        u8 mux = (1 << REFS0) | ((adcIndex[adcState]) & 0x0F);
        ADMUX = mux;
        PORTC = mux;
        ADCSRA |= (1 << ADSC);
        adcTrigger = True;*/

        // run HV loop
        if(eData.HVstep)
        {
            if(HTINPUT == 0)
            {
                gState.HVvalue += eData.HVstep;
            }else{
                gState.HVvalue -= eData.HVstep;
            }
            if(gState.HVvalue > 100) gState.HVvalue = 100;
            else if((signed)gState.HVvalue < 0) gState.HVvalue = 0;
            WritePWMValue(gState.HVvalue);
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
    gState.engine = STALLED;
}


/**
 * \fn ISR(TIMER1_COMPA_vect)
 * \brief Interrupt on compare of Timer1 channel A : ignition
 *
 * \param none
 * \return none
 */
ISR(TIMER1_COMPA_vect)
{
    if(curIgnTiming.state == OFF)
    {
        // TODO : use polarity
        C_SETBIT(IGNITION_PIN);
        curIgnTiming.state = ON;
        OCR1A = curIgnTiming.stop;
    }else{
        C_CLEARBIT(IGNITION_PIN);
        curIgnTiming.state = OFF;
        OCR1A = curIgnTiming.start;
    }
}

/**
 * \fn ISR(TIMER1_COMPB_vect)
 * \brief Interrupt on compare of Timer1 channel B : injection
 *
 * \param none
 * \return none
 */
ISR(TIMER1_COMPB_vect)
{
    if(curInjTiming.state == OFF)
    {
        // TODO : use polarity
        C_SETBIT(INJECTOR_PIN);
        curInjTiming.state = ON;
        OCR1B = curInjTiming.stop;
    }else{
        C_CLEARBIT(INJECTOR_PIN);
        curInjTiming.state = OFF;
        OCR1B = curInjTiming.start;
    }
}

/**
 * \fn void InitTimer(void)
 * \brief Init hardware and software timers
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
 * \fn void StartTimer(u8 timerHandle)
 * \brief start a software timer
 *
 * \param timerHandle handle on the timer to start
 * \return none
 *
 * To start timer, internal counter is set to master clock value
 */
void StartTimer(u8 timerHandle)
{
    ASSERT(timerHandle < TIMER_QTY);
    timerTable[timerHandle] = masterClk;
    return;
}


/**
 * \fn unsigned long GetTimer_ms(const u8 timerHandle)
 * \brief return number of milliseconds of timer defined by timerHandle
 *
 * \param timerHandle handle on the timer to get
 * \return number of milliseconds of timer defined by timerHandle
 *
 */
unsigned long GetTimer(const u8 timerHandle)
{
    ASSERT(timerHandle < TIMER_QTY);
    return masterClk - timerTable[timerHandle];
}


/**
 * \fn u8 EndTimer(const u8 timerHandle, const unsigned long duration)
 * \brief return if duration is over on timer defined by timerHandle
 *
 * \param timerHandle handle on the timer to get
 * \param duration wait time in msec
 * \return 0 if duration is not issued, 1 if issued
 *
 */
u8 EndTimer(const u8 timerHandle, const unsigned long duration)
{
    ASSERT(timerHandle < TIMER_QTY);
    if(masterClk - timerTable[timerHandle] > duration)
        return 1;
    else
        return 0;
}

/**
 * \fn void GetTime(u16 *dst)
 * \brief return the current time in second since start
 *
 * \param dst variable to hold the result
 * \return none
 *
 */
void GetTime(u16 *dst)
{
    *dst = (u16)(masterClk / 1000); 
    return;
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
    return;
    u8 mux;
    // save results
    adcValues[adcState] = ADC;
    // Next channel
    adcState++;
    // start ADC acquisition
    if(adcIndex[adcState] != ADC_IDLE)
    {
        mux = (1 << REFS0) | ((adcIndex[adcState]) & 0x0F);
        ADMUX = mux;
        PORTC = mux;
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
             (0 << ADIE) | 
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
    DIDR0 = 0x0E; // set ADC1..3 pin to ADC inputs
    adcTrigger = False; 
    adcState = ADC_IDLE;
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

    u8 mux;
    // Conversion and filtering
    switch(adcState)
    {
        case ADC_BATTERY:
            gState.battery = 150 * (u32)ADC / 1024 * eData.ratio[ADC_BATTERY] / 100;     
            adcState = ADC_TEMPMOTOR;
            break;

        case ADC_TEMPMOTOR:
            gState.tempMotor = (u32)ADC * 500 / 1024 * eData.ratio[ADC_TEMPMOTOR] / 100;     
            adcState = ADC_TEMPAIR;
            break;

        case ADC_TEMPAIR:
            gState.tempAir = (u32)ADC * 500 / 1024 * eData.ratio[ADC_TEMPAIR] / 100;     
            adcState = ADC_THROTTLE;
            break;

        case ADC_THROTTLE:
            gState.throttle = (eData.ratio[ADC_THROTTLE] * (u32)ADC) / 1024;     
            adcState = ADC_BATTERY;
            break;
        case ADC_IDLE:
            adcState = ADC_BATTERY;

        default:
            //ASSERT(0);
            adcState = ADC_BATTERY;
    }
    mux = (1 << REFS0) | (adcIndex[adcState] & 0x0F);
    ADMUX = mux;
    PORTC = mux;
    ADCSRA |= (1 << ADSC);
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
    gState.HVvalue = 0;
}

/**
 * \fn void WritePWMValue(u8 value)
 * \brief write value to PWM
 *
 * \param u8 value : value to set in %
 * \return none
*/
void WritePWMValue(u8 value)
{
    OCR0B = (u8)(value * 255 / 100); 
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
    RPMperiod = TCNT1;
    // clear timer for next period
    TCNT1 = 0;
    // compute RPM : tick is 4us
    gState.rpm = 60 * (250000 / RPMperiod);
    gState.engine = RUNNING;

    // update injection and ignition timings
    curIgnTiming = nextIgnTiming;
    curInjTiming = nextInjTiming;
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
    u32 period; // unit is 4us
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
    gState.speed = ((u32)eData.wheelSize * 25 * 3600) / period;
}


/**
 * \fn void SetInjectionTiming(u8 *force, u16 *duration)
 * \brief compute and set timing for injection
 *
 * \param u8 force : set to FORCEON or FORCEOFF to force output state
 * \param u16 duration : injection duration in us 
 * \return none
*/
void SetInjectionTiming(u8 force, u16 duration)
{
    if(force == FORCEON)
    {
        nextInjTiming.state = FORCEON;
    }
    else if(force == FORCEOFF)
    {
        nextInjTiming.state = FORCEOFF;
    }
    else
    {
        nextInjTiming.state = OFF;
    }

    // Convert angle to timer tick through RPM. Timer tick is 4us
    // TODO : use PMHOffset setting
    u16 angleTick = (u32)RPMperiod * eData.injAdv / 360;
    nextInjTiming.start = angleTick - (duration >> 3);
    nextInjTiming.stop  = angleTick + (duration >> 3);

    return;
}

/**
 * \fn void SetIgnitionTiming(u8 force, u8 advance);
 * \brief compute and set timing for ignition
 *
 * \param u8 force : set to FORCEON or FORCEOFF to force output state
 * \param u8 advance : angle before TDH to trigger spark 
 * \return none
*/
void SetIgnitionTiming(u8 force, u8 advance)
{
    if(force == FORCEON)
    {
        nextIgnTiming.state = FORCEON;
    }
    else if(force == FORCEOFF)
    {
        nextIgnTiming.state = FORCEOFF;
    }
    else
    {
        nextIgnTiming.state = OFF;
    }

    // Convert angle to timer tick through RPM. Timer tick is 4us
    // TODO : use PMHOffset setting
    nextIgnTiming.start = (u32)RPMperiod * advance / 360;;
    nextIgnTiming.stop  = nextIgnTiming.start + (eData.igniDuration >> 2);

    return;
}

