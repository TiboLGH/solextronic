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
 * \file sim_helpers.h
 * \brief Helpers function to increase usability of simavr 
 * \author Thibault Bouttevin
 * \date Feb 2016
 *
 */

#ifndef __SIM_HELPERS_H__
#define __SIM_HELPERS_H__

#include "sim_irq.h"
#include "sim_elf.h"


avr_irq_t*
helpers_register_symbol_irq(
		struct avr_t * avr,
        elf_firmware_t *f,
		const char *name,
        int size);

avr_irq_t*
helpers_register_debug_irq(
		struct avr_t * avr,
		uint32_t base_address,
		const char *name,
        int size);

avr_irq_t*
helpers_register_16bit_irq(
		struct avr_t * avr,
		uint32_t base_address,
		const char *name);
	
#endif
