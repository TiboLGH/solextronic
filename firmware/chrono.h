/***************************************************************************
 *   Copyright (C) 2012 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   www.legalethurlant.fr.st                                              *
 *                                                                         *
 *   This file is part of SolexTronic                                      *
 *                                                                         *
 *   Solextronic is free software; you can redistribute it and/or modify   *
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
 * \file chrono.h
 * \brief Manage the chronometer services.
 * \author Thibault Bouttevin
 * \date June 2015
 *
 * This file implements the management of the chronometer services including:
 *  - current lap time
 *  - average lap time
 *  - number of laps
 *  - total time
 *  - total length
 *  - max speed
 *  - average speed
 *  - ...
 */


#ifndef CHRONO_H
#define CHRONO_H

#include <stdint.h>
#include "common.h"

/* Public functions */
void ChronoInit(void);
void ChronoReset(void);
void ChronoTopLap(void);
void ChronoTop100ms(void);

void ChronoGetCurrentTime(char *dest);
void ChronoGetAvgLapTime(char *dest);
u16  ChronoGetAvgSpeed(void);
u8   ChronoGetLapNumber(void);

#endif // CHRONO_H
