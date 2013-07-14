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
 * \file HT_supply.c
 * \brief Flyback power supply simulation 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "sim_avr.h"
#include "sim_time.h"
#include "HT_supply.h"


const uint32_t max_voltage 		= 500;
const uint32_t period_us  		= 10 * 1000; // 10ms
const double   decrease_factor 	= 0.95;

static volatile uint32_t internal_voltage; 
static volatile uint32_t last_pwm; 

/*
 * called when pin to analyze toggle
 */
static void HT_supply_pwm_in_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	last_pwm = value;
}

/*
 * periodic computation of internal voltage and status pin
 */
static avr_cycle_count_t
periodic_tick(
		struct avr_t * avr,
        avr_cycle_count_t when,
        void * param)
{
	HT_supply_t * p = (HT_supply_t *) param;

	internal_voltage = 0.2 * (max_voltage * last_pwm) / 256. +
					   0.8 * (decrease_factor * internal_voltage);
	  						

	avr_raise_irq(p->irq + IRQ_HT_SUPPLY_INTERNAL_VOLTAGE, internal_voltage & 0xFFFF);
	avr_raise_irq(p->irq + IRQ_HT_SUPPLY_STATUS_OUT, (internal_voltage > p->threshold)?1:0);
	return when + avr_usec_to_cycles(avr, period_us);
}


static const char * irq_names[IRQ_HT_SUPPLY_COUNT] = {
		[IRQ_HT_SUPPLY_PWM_IN]     		 = "8<HT_supply.pwm_in",
		[IRQ_HT_SUPPLY_STATUS_OUT] 		 = "1>HT_supply.status_out",
		[IRQ_HT_SUPPLY_INTERNAL_VOLTAGE] = "16>HT_supply.internal_voltage",
};

void
HT_supply_init(
		struct avr_t * avr,
		HT_supply_t *p,
		const uint32_t threshold)
{
	p->avr = avr;
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_HT_SUPPLY_COUNT, irq_names);
	avr_irq_register_notify(p->irq + IRQ_HT_SUPPLY_PWM_IN, HT_supply_pwm_in_hook, p);
	p->threshold = threshold;

	// launch periodic task in charge to refresh internal voltage and status pin
	avr_cycle_timer_register_usec(avr, period_us, periodic_tick, p);

	internal_voltage 	= 0;
	last_pwm			= 0;
}
