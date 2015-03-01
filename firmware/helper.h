/***************************************************************************
 *   Copyright (C) 2012 by Thibault Bouttevin                              *
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
 * \file helper.h
 * \brief Helper functions for firmware and test bench
 * \author Thibault Bouttevin
 * \date July 2014
 *
 * This file includes functions for conversion, interpolation,
 * ... for both firmware and simulation.
 * Warning ! this file will be used on both the microcontroller and the simulation test
 * program : use only c99 code, nothing specific to a platform and be careful about alignment !
 *
 */

#ifndef HELPER_H
#define HELPER_H

#include "varDef.h"

u16 Interp2D(volatile u8 *table, u16 rpm, u8 load);
u8  Interp1D(volatile u8 table[TABSIZE][2], u8 adcVal);


#endif
