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
 * \file push_button.c
 * \brief Push button simulation
 * \author Thibault Bouttevin
 * \date June 2016
 *
 * This push button simulation is based on Simavr button.h/c
 * Add set state function and imput polarity for flexibility
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "push_button.h"

#define V(msg, ...) do{fprintf(stdout, "Push_button: "); fprintf(stdout, msg, ##__VA_ARGS__ );}while(0)
//#define V(msg, ...) do{}while(0)

static avr_cycle_count_t
button_auto_release(
		avr_t * avr,
		avr_cycle_count_t when,
		void * param)
{
	button_t * b = (button_t *)param;
    b->value = (b->value?0:1);
    b->value = BUTTON_RELEASED;
	avr_raise_irq(b->irq + IRQ_BUTTON_OUT, (b->polarity?(!b->value):b->value));
	V("button %s auto_release\n", b->name);
	return 0;
}

/*
 * button press with auto release. set the "pin" to zero and register a timer
 * that will reset it in a few usecs
 */
void
button_press_mom(
		button_t * b,
		uint32_t duration_usec)
{
	avr_cycle_timer_cancel(b->avr, button_auto_release, b);
    b->value = BUTTON_PRESSED;
	avr_raise_irq(b->irq + IRQ_BUTTON_OUT, (b->polarity?(!b->value):b->value));
	V("button %s pressed with auto release in %d us\n", b->name, duration_usec);
	// register the auto-release
	avr_cycle_timer_register_usec(b->avr, duration_usec, button_auto_release, b);
}

/*
 * button press : set the "pin" to pressed state depending on polarity
 */
void
button_press(
		button_t * b)
{
    b->value = BUTTON_PRESSED;
	avr_raise_irq(b->irq + IRQ_BUTTON_OUT, (b->polarity?(!b->value):b->value));
	V("button %s pressed\n", b->name);
}

/*
 * button release : set the "pin" to released state depending on polarity
 */
void
button_release(
		button_t * b)
{
    b->value = BUTTON_RELEASED;
	avr_raise_irq(b->irq + IRQ_BUTTON_OUT, (b->polarity?(!b->value):b->value));
	V("button %s released\n", b->name);
}

/*
 * Set button state
 */
void
button_set_state(
		button_t * b,
		uint8_t state)
{
	avr_raise_irq(b->irq + IRQ_BUTTON_OUT, (state?1:0));
    b->value = state;
}

void
button_init(
		avr_t *avr,
		button_t * b,
        uint8_t polarity,
		const char * name)
{
    strncpy(b->name, name, 64);
    b->name[63] = 0;
    const char *pName = &(b->name[0]);
	b->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_BUTTON_COUNT, &pName);
	b->avr = avr;
    b->polarity = polarity?1:0; 
    b->value = BUTTON_RELEASED;
	avr_raise_irq(b->irq + IRQ_BUTTON_OUT, (b->polarity?(!b->value):b->value));
    V("btn %s initialized : polarity %s, state %s\n", name, (b->polarity?"REVERSE":"NORMAL"), (b->value?"PRESSED":"RELEASED"));
}

