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
 * \file timing_analyzer.c
 * \brief Part to analyze output pin timing 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "sim_time.h"
#include "timing_analyzer.h"


#define V(msg, ...) fprintf(stdout, msg, ##__VA_ARGS__ )
//#define V(msg, ...) 


static const char * irq_names[IRQ_TIMING_ANALYZER_COUNT] = {
		[IRQ_TIMING_ANALYZER_IN] = "1<timing_analyzer.in",
		[IRQ_TIMING_ANALYZER_REF_IN] = "1<timing_analyzer.ref_in",
};


/*
 * called when pin to analyze toggle
 */
static void timing_analyzer_in_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	timing_analyzer_t * p = (timing_analyzer_t*)param;
	uint32_t ts = p->avr->cycle; 
    //V("IN at %u, value %d, irq->value %d\n", ts, value, irq->value);
	// get timestamp and direction
	if (!irq->value && value) {	// rising edge
        if(p->result.last_in_rising_ts)
        {
            if(p->result.discard)
            {
                p->result.discard--;
            }else{
                p->result.count++;
                p->result.period += avr_cycles_to_usec(p->avr, ts - p->result.last_in_rising_ts);
                if(p->result.last_ref_in_ts) p->result.rising_offset += avr_cycles_to_usec(p->avr, ts - p->result.last_ref_in_ts);
            }
            //V("Period %d, rising %d, count %d\n", avr_cycles_to_usec(p->avr, ts - p->result.last_in_rising_ts), avr_cycles_to_usec(p->avr, ts - p->result.last_ref_in_ts), p->result.count);
        }
        p->result.last_in_rising_ts = ts;
	}
	else if(irq->value && !value) {	// falling edge
        if(p->result.last_in_rising_ts)
        {
            if(!(p->result.discard))
            {
                if(p->result.last_ref_in_ts) p->result.falling_offset += avr_cycles_to_usec(p->avr, ts - p->result.last_ref_in_ts);
                p->result.high_duration  += avr_cycles_to_usec(p->avr, ts - p->result.last_in_rising_ts);
            }
            //V("Duration %d, falling %d\n", avr_cycles_to_usec(p->avr, ts - p->result.last_in_rising_ts), avr_cycles_to_usec(p->avr, ts - p->result.last_ref_in_ts));
        }
		p->result.last_in_falling_ts = ts;
	}
}

/*
 * called when ref pin toggle
 */
static void timing_analyzer_ref_in_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	timing_analyzer_t * p = (timing_analyzer_t*)param;
	uint32_t ts = p->avr->cycle; 
    //V("REF at %u irq->value %d value %d\n", ts, irq->value, value);
	// get timestamp and direction
	if (!irq->value && value) {	// rising edge
		p->result.last_ref_in_ts = ts;
        //V("REF at %u\n", ts);
	}
}


/*
 * Reset statistics
 * Discard first "discard" cycles from stats
 */
void 
timing_analyzer_reset(
        timing_analyzer_t *p, 
        const int discard)
{
    memset(&(p->result), 0, sizeof(timing_analyzer_result_t));    
	p->result.discard 			= discard;
}

void
timing_analyzer_init(
		struct avr_t * avr,
		timing_analyzer_t *p,
		const char *name)
{
	p->avr = avr;
	char *pName = &(p->name[0]);
	strncpy(pName, name, 256);
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_TIMING_ANALYZER_COUNT, irq_names);
	avr_irq_register_notify(p->irq + IRQ_TIMING_ANALYZER_IN, timing_analyzer_in_hook, p);
	avr_irq_register_notify(p->irq + IRQ_TIMING_ANALYZER_REF_IN, timing_analyzer_ref_in_hook, p);

    memset(&(p->result), 0, sizeof(timing_analyzer_result_t));    
}

/*
 * Called by test bench to get results
 * return the number of cycle analyzed (note only the last one metrics are reported)
 */
int
timing_analyzer_result(
	timing_analyzer_t *p,
	timing_analyzer_result_t* result)
{
	uint32_t cycles = p->result.count;
    if(cycles)
    {
	    p->result.rising_offset 	/= p->result.count;
	    p->result.falling_offset 	/= p->result.count;
	    p->result.period 			/= p->result.count;
	    p->result.high_duration 	/= p->result.count;
    }

	memcpy(result, &(p->result), sizeof(timing_analyzer_result_t));
	return cycles;
}
