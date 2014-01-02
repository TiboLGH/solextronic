/***************************************************************************
 *   Copyright (C) 2012 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   www.legalethurlant.fr.st                                              *
 *                                                                         *
 *   This file is part of Solextronic                                      *
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
 * \file common.h
 * \brief Includes common declarations and definition of the project
 * \author Thibault Bouttevin
 * \version 0.1
 * \date October 2012
 *
 * This file includes common declarations and definition of the project
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <varDef.h>

#define VERSION_SOFT_MAJOR       0
#define VERSION_SOFT_MINOR       1
#define VERSION_HARD             1

#define LOG(msg)  if(Flags.debug) printf(msg)
void __assert__(char *expr, char *filename, int linenumber); 
#define ASSERT(_expr) do {if (!(_expr)) __assert__(#_expr, __FILE__, __LINE__);} while (0);

#define True  (1)
#define False (0)

typedef enum
{
    LOW = 0,
    HIGH
}STATE;

typedef enum
{
    POL_IGNITION = 0,
    POL_INJECTION,
	POL_PUMP,
    POL_PMH,
	POL_QTY
}POL_SEL;

enum
{
    STOP = 0,
    CRANKING,
    RUNNING,
    ALARM,
    ERROR,
    STALLED
};




/**
 * \struct Flags_t
 * \brief Store current state of the system
 *
 * Flags stores variable of system state
 */
typedef struct {
   u8 newCmd;    	/*!< New command in uart buffer */
   u8 debug;      /*!< debug printf enable */
   u8 PMHEnable;  /*!< PMH enable */
   STATE PMHState;   /*!< PMH current state : active/inactive */
}Flags_t;


/**
 * \struct TimeStamp_t
 * \brief Timestamp for accurate timing
 *
 * Store master clock timestamp + timer tick
 * there are 250 ticks of 4 us per masterclk period
 */
typedef struct {
    u32 clk;
    u8  tick;
}TimeStamp_t; 

#endif
