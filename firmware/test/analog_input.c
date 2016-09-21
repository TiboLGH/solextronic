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
 * \file analog_input.c
 * \brief Analog generator simulation 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "avr_adc.h"
#include "analog_input.h"

//#define V(msg, ...) do{fprintf(stdout, "Analog_input: "); fprintf(stdout, msg, ##__VA_ARGS__ );}while(0)
#define V(msg, ...) do{}while(0)


static const char * irq_names[IRQ_ANALOG_INPUT_COUNT] = {
		[IRQ_ANALOG_INPUT_VALUE] 		 = "<analog_input.value",
};

/*
 * called when ADC conversion is called
 */
void analog_input_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	analog_input_t *p = (analog_input_t*)param;
    avr_adc_mux_t mux;
    memcpy(&mux, &value, sizeof(mux));
    for(int i = 0; i < p->nb_inputs; i++)
    {
        if(mux.src == p->adc_irq_index[i]){
            if(p->ramp[i]){
                p->progress[i] = p->avr->cycle - p->initTime[i];
                if(p->progress[i] > p->ramp[i]){ // ramp is done
                    p->value[i] = p->targetValue[i];
                }else{ // ramp in progress
                    p->value[i] = p->progress[i] *(int)(p->targetValue[i] - p->initValue[i]) / p->ramp[i] + p->initValue[i];
                }
            } 
            //V("index %d, value %.0f\n", mux.src, p->value[i]*1000);
            avr_raise_irq(avr_io_getirq(p->avr, AVR_IOCTL_ADC_GETIRQ, mux.src), (int)(p->value[i]*1000));
            return;
        }
    }
}

void
analog_input_init(
		struct avr_t * avr,
		analog_input_t *p,
        const int nb_input,
		const int *adc_irq_index,
		const float *value)
{
    if(nb_input > MAX_ADC) return;
    p->avr = avr;
    p->nb_inputs = nb_input;
    for(int i = 0; i < nb_input; i++)
    {
        p->adc_irq_index[i] = adc_irq_index[i];
        p->value[i] = value[i];
        p->ramp[i] = 0;
        p->progress[i] = 0;
        V("index %d, adc %d, value %f\n", i, p->adc_irq_index[i], p->value[i]*1000);
    }
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_ANALOG_INPUT_COUNT, irq_names);
    avr_irq_t * i_adc = avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_OUT_TRIGGER);
	avr_irq_register_notify(i_adc, analog_input_hook, p);	
}

void
analog_input_set_value(
		analog_input_t *p,
        const int adc_index,
		const float value,
        const uint32_t ramp)
{
    if(adc_index >= p->nb_inputs)
    {
        printf("ADC setting out of range\n");
        return;
    }
    p->ramp[adc_index] = ramp;
    if(!p->ramp[adc_index]) // immediate application
    {
        p->value[adc_index] = value;
    }else{ // linear ramping from current value to target one
        p->progress[adc_index] = 0;
        p->targetValue[adc_index] = value;
        p->initValue[adc_index] = p->value[adc_index];
        p->initTime[adc_index] = p->avr->cycle;
    }
    V("Index %d = %f, ramp %u\n", adc_index, p->value[adc_index], p->ramp[adc_index]);
}
