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
 * \date February 2012
 *
 * This file implements low-level services and hardware interfaces including
 * USART emission/reception, EEPROM management, software timers and SPI bus.
 * It is targetted for ATMega328p on Arduino Nano board.
 *
 */


#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>


enum
{
   TIMER_BASE = 0,   /*!< Master clock timer : 1ms */
   TIMER_100MS,      /*!< 100 msec based timer */
   TIMER_QTY         /*!< timer quantity */
};

enum
{
    AUTO = 0,  /*!< mode auto */
    ON,        /*!< ON state */
    OFF,       /*!< OFF state */
    FORCEON,   /*!< force ON state : always on */
    FORCEOFF,  /*!< force OFF state : always off */
};

enum
{
    ADC_BATTERY = 0, /*!< ADC1 */
    ADC_CLT,         /*!< ADC3 */
    ADC_IAT,         /*!< ADC6 */
    ADC_TPS,         /*!< ADC2*/
    ADC_MAP,         /*!< ADC7 */
    ADC_IDLE
};

/* Pinout Definition */
// from AVR035: Efficient C Coding for AVR 

#define SETBIT(ADDRESS,BIT) (ADDRESS |= (1<<BIT)) 
#define CLEARBIT(ADDRESS,BIT) (ADDRESS &= ~(1<<BIT)) 
#define FLIPBIT(ADDRESS,BIT) (ADDRESS ^= (1<<BIT)) 
#define CHECKBIT(ADDRESS,BIT) (ADDRESS & (1<<BIT)) 

#define SETBITMASK(x,y) (x |= (y)) 
#define CLEARBITMASK(x,y) (x &= (~y)) 
#define FLIPBITMASK(x,y) (x ^= (y)) 
#define CHECKBITMASK(x,y) (x & (y)) 

#define VARFROMCOMB(x, y) x 
#define BITFROMCOMB(x, y) y 

#define C_SETBIT(comb) SETBIT(VARFROMCOMB(comb), BITFROMCOMB(comb)) 
#define C_CLEARBIT(comb) CLEARBIT(VARFROMCOMB(comb), BITFROMCOMB(comb)) 
#define C_FLIPBIT(comb) FLIPBIT(VARFROMCOMB(comb), BITFROMCOMB(comb)) 
#define C_CHECKBIT(comb) CHECKBIT(VARFROMCOMB(comb), BITFROMCOMB(comb))
#define sbi(a, b) (a) |= (1 << (b)) 
#define cbi(a, b) (a) &= ~(1 << (b)) 

// Arduino Nano LED on PB5 (pin 17 on 32 pins parts)
#define LED_DDR		DDRB
#define LED_PORT	PORTB
#define LED_PIN  PORTB, 5 
// Injector control on PB2/OC1B (pin 14 on 32 pins parts)
#define INJECTOR_PIN  PORTB, 2 
// Ignition control on PB1/OC1A (pin 13 on 32 pins parts)
#define IGNITION_PIN  PORTB, 1 
// Pump control on PD7 (pin 11 on 32 pins parts)
#define PUMP_PIN  PORTD, 7 

#define DEBUG TWBR
#define PARAM TWDR

/**
 * \struct wvf_t
 * \brief waveform generator configuration
 *
 * Store the timing of ON and OFF states for injection and ignition
 */
typedef struct {
    u8  state;
    u16 start;      // absolute date to next ON period 
    u16 duration;   // duration of on period
}wvf_t; 

/* USART related functions */
void InitUart(void);
#define USART_RX_EN  (UCSR0B |= _BV(RXCIE0))
#define USART_RX_DIS (UCSR0B &= ~_BV(RXCIE0))
#define USART_TX_EN  (UCSR0B |= _BV(TXCIE0))
#define USART_TX_DIS (UCSR0B &= ~_BV(TXCIE0))
void printstr(char *s);
void putchr(char c);

/* GPIOs functions */
void InitIOs(void);

/* EEPROM related functions */
void InitEeprom(u8 force);
void updateEeprom(void);


/* Software Timers related functions */
#define INJ_INT_ENABLE  (TIMSK1 |= _BV(OCIE1B))
#define INJ_INT_DISABLE (TIMSK1 &= ~_BV(OCIE1B))
#define IGN_INT_ENABLE  (TIMSK1 |= _BV(OCIE1A))
#define IGN_INT_DISABLE (TIMSK1 &= ~_BV(OCIE1A))
void InitTimer(void);
void StartTimer(u8 timerHandle);
u32 GetTimer(const u8 timerHandle);
u8 EndTimer(const u8 timerHandle, const u32 duration);
void GetTime(u16 *dst);

/* ADC related functions */
void ADCInit(void);
void ADCProcessing(void);
void startAdc(void);

/* PWM related functions */
void InitPWM(void);
void WritePWMValue(u8 value);

/* Injection and ignition timings settings */
#define INJ_TEST_ADV 100
#define IGN_TEST_ADV 100
void SetInjectionTiming(u8 force, u16 duration);
void SetIgnitionTiming(u8 force, u8 advance);
void InjectorStartTest(void);
void IgnitionStartTest(void);
void IgnitionStopTest(void);

#endif // PLATFORM_H
