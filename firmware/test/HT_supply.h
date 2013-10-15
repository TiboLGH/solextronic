
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
 * \file HT_supply.h
 * \brief Flyback power supply simulation 
 * \author Thibault Bouttevin
 * \date June 2013
 *
 */
#ifndef __HT_SUPPLY_H__
#define __HT_SUPPLY_H__

#include "sim_irq.h"

/*
 * This part is a simulated high voltage supply controlled by the AVR.
 * Input is a PWM signal, output is simply a single pin set at hi when internal 
 * voltage is higher than a given threshold
 * Dynamic model is really dumb (but enough to exercise AVR code) :
 * V(int_N) = (PWM / 256) x 500v x 20% - V(int_N-1) x 95%
 * Periodicity is 10ms
 */
enum {
	IRQ_HT_SUPPLY_PWM_IN = 0,	
	IRQ_HT_SUPPLY_STATUS_OUT,	
	IRQ_HT_SUPPLY_INTERNAL_VOLTAGE,	// to trace internal voltage in VCD for example	
	IRQ_HT_SUPPLY_COUNT
};

typedef struct HT_supply_t {
	avr_irq_t *	irq;		// irq list
	struct avr_t * avr;		// avr instance for current time
	uint32_t threshold;		// threshold for "power good" status
} HT_supply_t;

void
HT_supply_init(
		struct avr_t * avr,
		HT_supply_t *p,
		const uint32_t threshold);

#endif
