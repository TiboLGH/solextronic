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
 * \file timing_analyzer.h
 * \brief Part to analyze output pin timing 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 */

#ifndef __TIMING_ANALYZER_H__
#define __TIMING_ANALYZER_H__

#include "sim_irq.h"

/*
 * This part is a timing analyzer. Based on reference signal rising edge, it
 * compute several parameters of an output signal (supposed sync with reference).
 */
enum {
	IRQ_TIMING_ANALYZER_IN = 0,	
	IRQ_TIMING_ANALYZER_REF_IN,	
	IRQ_TIMING_ANALYZER_COUNT
};

typedef struct timing_analyzer_result_t {
	uint32_t	rising_offset;	// rising edge offset from REF rising edge
	uint32_t	falling_offset;	// falling edge offset from REF rising edge
	uint32_t 	period;			// period on rising edge
	uint32_t 	high_duration;	// high state duration
	uint32_t 	count;	        // count of cycle
} timing_analyzer_result_t;

typedef struct timing_analyzer_t {
	avr_irq_t *	irq;		// irq list
	struct avr_t * avr;		// avr instance for current time
	char		name[256];	// analyzer name, to use multiple instances
	timing_analyzer_result_t  	result;		// analysis result
} timing_analyzer_t;



void
timing_analyzer_init(
		struct avr_t * avr,
		timing_analyzer_t *p,
		const char *name);

int
timing_analyzer_result(
	timing_analyzer_t *p,
	timing_analyzer_result_t* result);
		
#endif
