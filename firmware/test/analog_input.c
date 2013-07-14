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
#include "analog_input.h"


static const char * irq_names[IRQ_ANALOG_INPUT_COUNT] = {
		[IRQ_ANALOG_INPUT_VALUE] 		 = "<analog_input.value",
};

void
analog_input_init(
		struct avr_t * avr,
		analog_input_t *p,
		const float value,
		const char *name)
{
	char *pName = &(p->name[0]);
	strncpy(pName, name, 256);
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_ANALOG_INPUT_COUNT, irq_names);
	p->value = value;
	avr_raise_irq(p->irq + IRQ_ANALOG_INPUT_VALUE, value);
}

void
analog_input_set_value(
		analog_input_t *p,
		const float value)
{
	p->value = value;
	avr_raise_irq(p->irq + IRQ_ANALOG_INPUT_VALUE, value);
}
