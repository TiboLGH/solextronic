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

//#define V(msg, ...) do{fprintf(stdout, "Pulse_input: "); fprintf(stdout, msg, ##__VA_ARGS__ );}while(0)
#define V(msg, ...) do{}while(0)

static avr_cycle_count_t
switch_auto(
		struct avr_t * avr,
        avr_cycle_count_t when,
        void * param)
{
	pulse_input_t * b = (pulse_input_t *) param;
	if(b->high){
        b->value = !b->value;
        //if(b->value) V("Pulse %s cyle %llu\n", b->name, avr->cycle);
        avr_raise_irq(b->irq + IRQ_PULSE_OUT, b->value);
    }
    if(!b->ramp){ //static
        if(b->value){
            return when + avr_usec_to_cycles(avr, b->high);
        }else{
            return when + avr_usec_to_cycles(avr, b->low);
        }
    }
    else{
        b->progress = b->avr->cycle - b->initTime;
        if(b->progress > avr_usec_to_cycles(b->avr, b->ramp)){ // end of ramp
            b->ramp = 0;
            V("End of ramp. Low = %u\n", b->targetLow);
            if(b->value){
                return when + avr_usec_to_cycles(avr, b->targetHigh);
            }else{
                return when + avr_usec_to_cycles(avr, b->targetLow);
            }
        }else{ // ramp in progress
            if(b->value){
                b->high = b->progress*(int)(b->targetHigh - b->initHigh) / avr_usec_to_cycles(b->avr, b->ramp) + b->initHigh;
                return when + avr_usec_to_cycles(avr, b->high);
            }else{
                b->low = b->progress*((double)b->targetLow - b->initLow) / avr_usec_to_cycles(b->avr, b->ramp) + b->initLow;
                V("Progress %u, target %u\n", b->progress, avr_usec_to_cycles(b->avr, b->ramp)); 
                V("Cur %u, target/init : %u/%u\n", b->low, b->targetLow, b->initLow);
                return when + avr_usec_to_cycles(avr, b->low);
            }
        }
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
    b->ramp = 0;
    b->progress = 0;
	avr_cycle_timer_register_usec(avr, b->low, switch_auto, b);
	V("init %s period %duS, duty cycle %.1f%%\n", b->name, tHigh+tLow, 100*(float)tHigh/(tHigh+tLow));
}

void pulse_input_config(
        pulse_input_t *b, 
        const uint32_t tHigh, 
        const uint32_t tLow,
        const uint32_t ramp)
{
    int isStopped = (!(b->high) || !(b->low));
    b->ramp = ramp * 1000; // computations are done in us
    if(!b->ramp) // immediate application
    {
        b->low = tLow;
        b->high = tHigh;
    }else{ // linear ramping from current value to target one
        b->targetHigh = tHigh;
        b->targetLow  = tLow;
        b->initHigh   = b->high;
        b->initLow    = b->low;
        b->progress   = 0;
        b->initTime   = b->avr->cycle; 
    }
    if(!tLow || !tHigh) // stop generator, set ouput to low
    {
        avr_cycle_timer_cancel(b->avr, switch_auto, b);
        b->value = 0;
        avr_raise_irq(b->irq + IRQ_PULSE_OUT, b->value);
        V("Generator %s stopped\n", b->name);
    }else{
        V("config %s period %duS, duty cycle %.1f%%, ramp %d\n", b->name, tHigh+tLow, 100*(float)tHigh/(tHigh+tLow), ramp);
        if(isStopped) // generator was stopped, explicitely restart timer
            avr_cycle_timer_register_usec(b->avr, b->low, switch_auto, b);
    }
}
