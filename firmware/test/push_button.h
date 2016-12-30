/***************************************************************************
 *   Copyright (C) 2016 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   https://github.com/TiboLGH/solextronic                                *
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
 * \file push_button.h
 * \brief Push button simulation
 * \author Thibault Bouttevin
 * \date June 2016
 *
 * This push button simulation is based on Simavr button.h/c
 * Add set state function and input polarity for flexibility
 *
 */
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "sim_irq.h"

enum {
	IRQ_BUTTON_OUT = 0,
	IRQ_BUTTON_COUNT
};

enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
};

enum {
    BUTTON_NORMAL = 0,
    BUTTON_REVERSE
};

typedef struct button_t {
	avr_irq_t * irq;	// output irq
	struct avr_t * avr;
    char name[64];
    int logHandle;
	uint8_t value;
    uint8_t polarity;
} button_t;

void
button_init(
		struct avr_t * avr,
		button_t * b,
        uint8_t polarity, /* 0 : normal, 1 : reverse */
		const char * name);

void
button_press(
		button_t * b);

void
button_release(
		button_t * b);

void
button_press_mom(
		button_t * b,
		uint32_t duration_usec);
void
button_set_state(
		button_t * b,
		uint8_t state);

#endif /* __BUTTON_H__*/
