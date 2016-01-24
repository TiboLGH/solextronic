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
#include "helper.h"
#include "chrono.h"

#define BAUD 	        57600

eeprom_data_t EEMEM eeprom;
const PROGMEM eeprom_data_t eeInit = {
    .timerLed       = 20,
    .wheelSize      = 182,
    .PMHOffset      = 0,
    .maxRPM         = 0,
    .maxTemp        = 150,
    .minBat         = 110,
    .ignDuration    = 1000,
    .ignStarter     = 5,
    .ignOverheat    = 2,
    .noSparkAtDec   = 0,
    .injOpen        = 500,
    .injRate        = 500, //?
    .injAdv         = 140, //?
    .injStarter     = 1000,
    .injOverheat    = 3,
    .noInjAtDec     = 0,
    .injStart       = 500,
    .holdPWM        = 50,
    .ignPolarity    = 0,
    .injPolarity    = 0,
    .pmhPolarity    = 0,
    .pumpPolarity   = 0,
    .tpsMin         = 0,
    .tpsMax         = 255,
    .battRatio      = 160,
    .map0           = 0,
    .map5           = 110,
    .iatCal         = {[0][0]=   0,[0][1]=200/*deg*/,
                       [1][0]=  18,[1][1]=160/*deg*/,
                       [2][0]=  48,[2][1]=120/*deg*/,
                       [3][0]=  86,[3][1]= 90/*deg*/,
                       [4][0]= 121,[4][1]= 70/*deg*/,
                       [5][0]= 141,[5][1]= 60/*deg*/,
                       [6][0]= 181,[6][1]= 40/*deg*/,
                       [7][0]= 214,[7][1]= 20/*deg*/,
                       [8][0]= 236,[8][1]=  0/*deg*/,
                       [9][0]= 255,[9][1]=  0/* negative temp*/},
    .cltCal         = {[0][0]=   0,[0][1]=200/*deg*/,
                       [1][0]=  18,[1][1]=160/*deg*/,
                       [2][0]=  48,[2][1]=120/*deg*/,
                       [3][0]=  86,[3][1]= 90/*deg*/,
                       [4][0]= 121,[4][1]= 70/*deg*/,
                       [5][0]= 141,[5][1]= 60/*deg*/,
                       [6][0]= 181,[6][1]= 40/*deg*/,
                       [7][0]= 214,[7][1]= 20/*deg*/,
                       [8][0]= 236,[8][1]=  0/*deg*/,
                       [9][0]= 255,[9][1]=  0/* negative temp*/},
    /* default table from Shadocs : http://www.solex-competition.net/forum/garage.php?mode=view_image&image_id=1537 */
    .ignTable       = {[0][0]= 10, [0][1]= 10, [0][2]= 10, [0][3]= 10, [0][4]= 10, [0][5]= 10, [0][6]= 10, [0][7]= 10, [0][8]= 10, [0][9]= 10, /*1000rpm*/
                       [1][0]= 10, [1][1]= 10, [1][2]= 10, [1][3]= 10, [1][4]= 10, [1][5]= 10, [1][6]= 10, [1][7]= 10, [1][8]= 10, [1][9]= 10, /*2000rpm*/
                       [2][0]= 14, [2][1]= 14, [2][2]= 14, [2][3]= 14, [2][4]= 14, [2][5]= 14, [2][6]= 14, [2][7]= 14, [2][8]= 14, [2][9]= 14, /*3000rpm*/
                       [3][0]= 19, [3][1]= 19, [3][2]= 19, [3][3]= 19, [3][4]= 19, [3][5]= 19, [3][6]= 19, [3][7]= 19, [3][8]= 19, [3][9]= 19, /*4000rpm*/
                       [4][0]= 22, [4][1]= 22, [4][2]= 22, [4][3]= 22, [4][4]= 22, [4][5]= 22, [4][6]= 22, [4][7]= 22, [4][8]= 22, [4][9]= 22, /*5000rpm*/
                       [5][0]= 23, [5][1]= 23, [5][2]= 23, [5][3]= 23, [5][4]= 23, [5][5]= 23, [5][6]= 23, [5][7]= 23, [5][8]= 23, [5][9]= 23, /*6000rpm*/
                       [6][0]= 22, [6][1]= 22, [6][2]= 22, [6][3]= 22, [6][4]= 22, [6][5]= 22, [6][6]= 22, [6][7]= 22, [6][8]= 22, [6][9]= 22, /*7000rpm*/
                       [7][0]= 20, [7][1]= 20, [7][2]= 20, [7][3]= 20, [7][4]= 20, [7][5]= 20, [7][6]= 20, [7][7]= 20, [7][8]= 20, [7][9]= 20, /*8000rpm*/
                       [8][0]= 16, [8][1]= 16, [8][2]= 16, [8][3]= 16, [8][4]= 16, [8][5]= 16, [8][6]= 16, [8][7]= 16, [8][8]= 16, [8][9]= 16, /*9000rpm*/
                       [9][0]= 10, [9][1]= 10, [9][2]= 10, [9][3]= 10, [9][4]= 10, [9][5]= 10, [9][6]= 10, [9][7]= 10, [9][8]= 10, [9][9]= 10, /*10000rpm*/},
};

extern volatile eeprom_data_t    eData;
extern volatile Current_Data_t   gState;
extern volatile intState_t       intState;

extern u8 *bufTx;
extern u8 bufRx[];
extern u8 indexRx;
extern u16 txCur;
extern u16 txCount;
extern u8 isTx;

static volatile u8 adcState;
static u32	timerTable[TIMER_QTY];
static volatile u32 masterClk;
static volatile u8 count10ms, count100ms;
static TimeStamp_t     prevTs, newTs;
static volatile wvf_t curInjTiming = {.state = OFF, .start = 0, .duration = 0};
static volatile wvf_t nextInjTiming = {.state = OFF, .start = 0, .duration = 0};
static volatile wvf_t curIgnTiming = {.state = OFF, .start = 0, .duration = 0};
static volatile wvf_t nextIgnTiming = {.state = OFF, .start = 0, .duration = 0};

const u8 adcIndex[] = {1, 3, 6, 2, 7, 255}; 

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
        intState.rReady = 1;
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
    /*
     * PORT B :
     *  Aux on PB0 : input
     *  Ignition on PB1 : output
     *  Injection on PB2 : output
     *  LED on PB5 : output
     * Port D :
     *  Dead high on PD2/INT0 : input, pull-up, interrupt on rising edge
     *  Wheel synchro on PD3/INT1 : input, pull-up, interrupt on rising edge 
     *  BTN1/Cranking on PD4 : input, pull-up 
     *  HV supply on PD5 : output
     *  Pump on PD7 : output
     */

    DDRB  = 0b00100110;
    PORTB = 0b00000000;
    DDRD  = 0b11100010;
    PORTD = 0b00010000;

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
    /* read first byte to check eeprom content : if 0xFF, eeprom is void */
    if(force || (255 == eeprom_read_byte(&eeprom.wheelSize)))
    {
        /* initialise eeprom */
        memcpy_P((void *)&eData, (PGM_VOID_P)&eeInit, sizeof(eeprom_data_t));
        for(u8 i = 0; i < TABSIZE; i++)
        {
            eData.rpmBins[i]  = 1000 * (i+1);
            eData.loadBins[i] = 10 * (i+1);
        }
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
    static u16 index = 0;
    u8 *pData = (u8 *)&(eData.wheelImpulse);
    if(eeprom_is_ready())
    {
        eeprom_update_byte((u8*)(&(eeprom.wheelImpulse) + index), *(pData + index));
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
    count100ms++;
    if(count10ms >= 10)
    {
        count10ms = 0;
        // Injector test mode : simulate a INT0 input each 10ms
        if(gState.injTestMode)
        {
            if(!gState.injTestCycles) // end of test
            {
                INJ_INT_DISABLE;
                gState.injTestMode = False;
                gState.engineState = M_STOP;
            }else{ // trigger cycle     
                INJ_INT_DISABLE;
                curInjTiming.start = TCNT1 + nextInjTiming.start;
                curInjTiming.duration  = nextInjTiming.duration;
                curInjTiming.state = OFF;
                PIN_OFF(INJECTOR_PIN, eData.injPolarity);
                OCR1B = curInjTiming.start;
                INJ_INT_ENABLE;
            }
            gState.injTestCycles--;        
        }
        
        // Ignition test mode
        if(intState.ignTestMode)
        {
            IGN_INT_DISABLE;
            curIgnTiming.start = TCNT1 + nextIgnTiming.start;
            curIgnTiming.duration  = nextIgnTiming.duration;
            PIN_OFF(IGNITION_PIN, eData.ignPolarity);
            curIgnTiming.state = OFF;
            OCR1A = curIgnTiming.start;
            IGN_INT_ENABLE;
        }

        // start ADC acquisition
        startAdc();
    }
    
    if(count100ms >= 100)
    {
        count100ms = 0;
        ChronoTop100ms();
    }
}
        
void startAdc(void)
{
    // start ADC acquisition
    adcState = ADC_BATTERY;
    ADMUX = (1 << REFS0) | (1 << ADLAR) | ((adcIndex[adcState]) & 0x0F);
    ADCSRA |= (1 << ADSC);
    intState.adcDone = False;
}

/**
 * \fn ISR(TIMER1_OVF_vect)
 * \brief FIXME Interrupt on overflow of Timer1 : engine is stalled
 *
 * \param none
 * \return none
 */
ISR(TIMER1_OVF_vect)
{
    if(gState.engineState == M_RUNNING) gState.engineState = M_STALLED;
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
        PIN_ON(IGNITION_PIN, eData.ignPolarity);
        curIgnTiming.state = ON;
        OCR1A += curIgnTiming.duration;
    }else{
        PIN_OFF(IGNITION_PIN, eData.ignPolarity);
        curIgnTiming.state = OFF;
        if(intState.ignTestMode)
            IGN_INT_DISABLE;
        else
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
        PIN_ON(INJECTOR_PIN, eData.injPolarity);
        curInjTiming.state = ON;
        OCR1B += curInjTiming.duration;
    }else{
        PIN_OFF(INJECTOR_PIN, eData.injPolarity);
        curInjTiming.state = OFF;
        if(gState.injTestMode) 
            INJ_INT_DISABLE;
        else
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
    TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
    TCCR1B = (0 << ICNC1 ) | (1 << ICES1 ) | (0 << WGM13 ) | (0 << WGM12 ) | (0 << CS12 ) | (1 << CS11 ) | (1 << CS10);
    TCNT1 = 0;
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
    // save results, use only the MSB bits. Small WA as tunerstudio doesn't support array in outputchannel
    *(&(gState.rawBattery) + adcState) = ADCH;
    // Next channel
    adcState++;
    if(adcState != ADC_IDLE)
    {
        ADMUX = (1 << REFS0) | (1 << ADLAR) | ((adcIndex[adcState]) & 0x0F);
        ADCSRA |= (1 << ADSC);
    }else{
        intState.adcDone = True;
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
             (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // prescaler 64 : 250kHz for 16MHz Xtal, 52us per conversion
    DIDR0 = 0x0E; // set ADC1..3 pin to ADC inputs
    ADMUX = (1 << REFS0) | (1 << ADLAR) | (adcIndex[ADC_BATTERY] & 0x0F);
    intState.adcDone = False; 
    adcState = ADC_IDLE;
}

/**
 * \fn void ADCProcessing(void)
 * \brief convert ADC captures
 *
 * \param none
 * \return none
*/
void ADCProcessing(void)
{
    static u8 lastTps = 0;

    if(adcState != ADC_IDLE) return;

    // Conversion and filtering
    gState.battery = (eData.battRatio * (u16)gState.rawBattery) / 256 + 7; // + 0.7v for the diode     
    gState.CLT = Interp1D(eData.cltCal, gState.rawClt);     
    gState.IAT = Interp1D(eData.iatCal, gState.rawIat);     
    gState.TPS = 100 * (u16)(gState.rawTps - eData.tpsMin) / (eData.tpsMax - eData.tpsMin); 
    gState.TPSVariation = lastTps - gState.TPS;
    lastTps = gState.TPS;    
    if(gState.TPS > 97) gState.TPSState = T_WOT;
    else if(gState.TPS < 3) gState.TPSState = T_IDLE;
    else gState.TPSState = T_NORMAL;
    gState.MAP = (u16)gState.rawMap * (eData.map5 - eData.map0) / 255 - eData.map0; 
    // conversion done, inhibit useless recompute until next acquisition
    intState.adcDone = False;
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
    static u16 lastTick = 0;
    // Save timer value
    u16 latchedTimer1 = TCNT1;
    // Compute RPM period with predictive algo
    if(latchedTimer1 < lastTick) // overflow of Timer1
    {
        intState.period_1 = 65535 - lastTick + latchedTimer1;
    }else{
        intState.period_1 = latchedTimer1 - lastTick;
    } 
    lastTick = latchedTimer1;

	intState.RPMperiod = intState.period_1 * 2 - intState.period_2;
    intState.period_2 = intState.period_1;

    // compute RPM : tick is 4us
    gState.rpm = (60 * 250000) / intState.RPMperiod;

    // update injection and ignition timings
    if((gState.engineState == M_CRANKING) ||
       (gState.engineState == M_RUNNING))
      {
          INJ_INT_DISABLE;
          curInjTiming.start = latchedTimer1 + nextInjTiming.start;
          curInjTiming.duration  = nextInjTiming.duration;
          if(curInjTiming.state == OFF) OCR1B = curInjTiming.start;
          INJ_INT_ENABLE;

          IGN_INT_DISABLE;
          curIgnTiming.start = latchedTimer1 + nextIgnTiming.start;
          curIgnTiming.duration  = nextIgnTiming.duration;
          if(curIgnTiming.state == OFF) OCR1A = curIgnTiming.start;
          IGN_INT_ENABLE;
      }
    
    // Flag to compute a new cycle
    intState.newCycle = 1;
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
    // wh/period = cm/4us
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
    // Convert angle to timer tick through RPM. Timer tick is 4us
	// This define the start of injection
    // TODO : use PMHOffset setting
    u16 angleTick = (u32)intState.RPMperiod * (360 - eData.injAdv) / 360;
    nextInjTiming.start = angleTick;
    nextInjTiming.duration  = duration >> 2; // us to 4us unit
    
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
    // Convert angle to timer tick through RPM. Timer tick is 4us
    // TODO : use PMHOffset setting
    nextIgnTiming.start = (u32)intState.RPMperiod * (360 - advance) / 360;
    nextIgnTiming.duration  = eData.ignDuration >> 2;

    return;
}

/**
 * \fn void InjectorStartTest(void)
 * \brief Start injector test mode
 *
 * \return none
*/
void InjectorStartTest(void)
{
    /* Configure waveform generator */
    IGN_INT_DISABLE;
    INJ_INT_DISABLE;
    /* Configure waveform generator */
    nextInjTiming.start = INJ_TEST_ADV;
    nextInjTiming.duration  = eData.injTestPW >> 2; // conversion from us to timer step (4us)
    gState.injTestCycles--;
    INJ_INT_ENABLE;
    gState.injTestMode = True;
    return;
}

/**
 * \fn void IgnitionStartTest(void)
 * \brief Start ignition test mode
 *
 * \return none
*/
void IgnitionStartTest(void)
{
    IGN_INT_DISABLE;
    INJ_INT_DISABLE;
    /* Configure waveform generator */
    nextIgnTiming.start = IGN_TEST_ADV;
    nextIgnTiming.duration  = eData.ignDuration >> 2; // conversion from us to timer step (4us) 
    intState.ignTestMode = True;
    IGN_INT_ENABLE;
    return;
}

/**
 * \fn void IgnitionStopTest(void)
 * \brief Stop ignition test mode
 *
 * \return none
*/
void IgnitionStopTest(void)
{
    intState.ignTestMode = False;
    IGN_INT_DISABLE;
    return;
}
