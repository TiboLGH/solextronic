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
 * \file analog_input.h
 * \brief Analog generator simulation 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 */

#ifndef __ANALOG_INPUT_H__
#define __ANALOG_INPUT_H__

#include "sim_irq.h"

#define MAX_ADC 10

/*
 * This part is a anlog input driver
 */
enum {
	IRQ_ANALOG_INPUT_VALUE = 0,	
	IRQ_ANALOG_INPUT_COUNT
};

typedef struct analog_input_t {
	avr_irq_t *	irq;		          // irq list
	struct avr_t * avr;		          // avr instance for current time
    int       nb_inputs;              // number of inputs 
	float	  value[MAX_ADC];		  // value in v
	int 	  adc_irq_index[MAX_ADC]; // adc channel
    uint32_t  ramp[MAX_ADC];          // ramp duration
    uint32_t  progress[MAX_ADC];      // current progress in ramp
    uint64_t  initTime[MAX_ADC];      // start time for ramp
	float	  initValue[MAX_ADC];	  // initial value of ramp in v
	float	  targetValue[MAX_ADC];	  // target value of ramp in v
} analog_input_t;

void
analog_input_init(
		struct avr_t * avr,
		analog_input_t *p,
        const int nb_input,
		const int *adc_irq_index,
		const float *value);

void
analog_input_set_value(
		analog_input_t *p,
        const int adc_index,
		const float value,
        const uint32_t ramp);

#endif
