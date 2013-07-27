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
 * \file pulse_input.c
 * \brief Pulse input is used to simulate a pulse
 * \author Thibault Bouttevin
 * \date June 2013
 *
 * This generator is based on simavr AC input
 *
 */

#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "sim_time.h"
#include "pulse_input.h"


static avr_cycle_count_t
switch_auto(
		struct avr_t * avr,
        avr_cycle_count_t when,
        void * param)
{
	pulse_input_t * b = (pulse_input_t *) param;
	b->value = !b->value;
	avr_raise_irq(b->irq + IRQ_PULSE_OUT, b->value);
    if(b->value){
        //avr_raise_irq(b->irq + IRQ_PULSE_RISING, b->value);
        return when + avr_usec_to_cycles(avr, b->high);
    }else{
        //avr_raise_irq(b->irq + IRQ_PULSE_FALLING, b->value);
        return when + avr_usec_to_cycles(avr, b->low);
    }
}


void 
pulse_input_init(
        avr_t *avr, 
        pulse_input_t *b, 
        const char *name,
        const uint32_t tHigh, 
        const uint32_t tLow)
{
    strncpy(b->name, name, 64);
    b->name[63] = 0;
    const char *pName = &(b->name[0]);
	b->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_PULSE_COUNT, &pName);
	b->avr = avr;
	b->value = 0;
    b->low = tLow;
    b->high = tHigh;
	avr_cycle_timer_register_usec(avr, b->low, switch_auto, b);
	//printf("pulse_input_init period %duS, duty cycle %.1f%%\n", tHigh+tLow, 100*(float)tHigh/(tHigh+tLow));
}

void pulse_input_config(
        pulse_input_t *b, 
        const uint32_t tHigh, 
        const uint32_t tLow)
{
    b->low = tLow;
    b->high = tHigh;
	//printf("pulse_input_init period %duS, duty cycle %.1f%%\n", tHigh+tLow, 100*(float)tHigh/(tHigh+tLow));
}
