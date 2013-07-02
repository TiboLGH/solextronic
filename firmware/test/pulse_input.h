/***************************************************************************
 *   Copyright (C) 2013 by Thibault Bouttevin                              *
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
 * \file pulse_input.h
 * \brief Pulse input is used to simulate a pulse
 * \author Thibault Bouttevin
 * \date June 2013
 *
 * This generator is based on simavr AC input
 *
 */
#ifndef __PULSE_INPUT_H__
#define __PULSE_INPUT_H__

#include "sim_irq.h"


enum {
    IRQ_PULSE_OUT = 0,
    IRQ_PULSE_COUNT
};

typedef struct pulse_input_t {
    avr_irq_t * irq;
    struct avr_t * avr;
    uint8_t value;
    char name[64];
    uint32_t high;
    uint32_t low;
} pulse_input_t;

void 
pulse_input_init(
        avr_t *avr, 
        pulse_input_t *b, 
        const char *name,
        const uint32_t tHigh, 
        const uint32_t tLow);

void pulse_input_config(
        pulse_input_t *b, 
        const uint32_t tHigh, 
        const uint32_t tLow);
#endif
