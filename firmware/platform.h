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
    ADC_BATTERY = 0, /*!< ADC7 */
    ADC_TEMPMOTOR,   /*!< ADC1 */
    ADC_TEMPAIR,     /*!< ADC2 */
    ADC_THROTTLE,    /*!< ADC3 */
    ADC_PRESSURE,    /*!< ADC6 */
    ADC_IDLE
};


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
void InitTimer(void);
void StartTimer(u8 timerHandle);
u32 GetTimer(const u8 timerHandle);
u8 EndTimer(const u8 timerHandle, const u32 duration);
void GetTime(u16 *dst);

/* ADC related functions */
void ADCInit(void);
void ADCProcessing(void);

/* PWM related functions */
void InitPWM(void);
void WritePWMValue(u8 value);

#endif // PLATFORM_H
