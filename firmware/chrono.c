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
 * \file chrono.c
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "chrono.h"
#include "common.h"


/*** System variables ***/
extern eeprom_data_t    eData;
extern Current_Data_t   gState;
extern intState_t       intState;

static u32 accLength = 0;   // meter
static u32 accTime = 0;     // 0.1sec unit
static u8 accLap = 0;
static u16 bestLapTime = 0; // 0.1sec unit
static u16 curLapTime = 0;  // 0.1sec unit
static bool enabled = false;


/*** Internal functions ***/



/*** Public functions ***/

/**
* ChronoInit
*
* Parameters:
*       none
* Description:
*		Init of the chrono services. To be called once at startup.
* Returns:
*		none
*/
void ChronoInit(void)
{
    // Reset all internal var
    ChronoReset();
}

/**
* ChronoReset
*
* Parameters:
*       none
* Description:
*		Reset internal accumulator of time and distance, disable service until lap top.
* Returns:
*		none
*/
void ChronoReset(void)
{
    accLength = 0;   // meter
    accTime = 0;     // 0.1sec unit
    accLap = 0;
    bestLapTime = 0; // 0.1sec unit
    curLapTime = 0;  // 0.1sec unit
    enabled = false;
    return;
}


/**
* ChronoTopLap
*
* Parameters:
*       none
* Description:
*		Called at the end of a lap. Save the current lap time and count.
*		If service is disabled, enabled it ( => start of race)
* Returns:
*		none
*/
void ChronoTopLap(void)
{
    enabled = true;
    accLap++;
    accTime += curLapTime;
    accLength += eData.lapLength;
    if((bestLapTime > curLapTime) || !bestLapTime) 
    {
        bestLapTime = curLapTime;
    }
    curLapTime = 0;
}

/**
* ChronoTop100ms
*
* Parameters:
*       none
* Description:
*		Called on 100ms tick. Update internal time.
* Returns:
*		none
*/
void ChronoTop100ms(void)
{
    if(enabled)
    {
        curLapTime++; // 0.1sec unit
    }
}

/**
* ChronoGetCurrentTime
*
* Parameters:
*       dest : string to store the result. Should be long enough.
* Description:
*		Return current lap time in "min'sec"1/10" format. For LCD typically
* Returns:
*		none
*/
void ChronoGetCurrentTime(char *dest)
{
    u8 tenth = curLapTime % 10;
    u8 sec   = (curLapTime/10) % 60;
    u8 min   = (curLapTime/600);
    sprintf(dest, "%2u'%2u\"%1u ", min, sec, tenth);
    return;
}

/**
* ChronoGetAvgLapTime
*
* Parameters:
*       dest : string to store the result. Should be long enough.
* Description:
*		Return avg lap time in "min'sec"1/10" format. For LCD typically
* Returns:
*		none
*/
void ChronoGetAvgLapTime(char *dest)
{
    if(!accLap)
    {
        sprintf(dest, " 0' 0\"0 ");
        return;
    }
    u16 avgLapTime = accTime / accLap;
    u8 tenth = avgLapTime % 10;
    u8 sec   = (avgLapTime/10) % 60;
    u8 min   = (avgLapTime/600);
    sprintf(dest, "%2u'%2u\"%1u ", min, sec, tenth);
    return;
}

/**
* ChronoGetAvgSpeed
*
* Parameters:
*       none
* Description:
*		Return avg speed over all laps in 1/10 of km/h
* Returns:
*	    speed in 1/10 km/h
*/
u16   ChronoGetAvgSpeed(void)
{
    if(!accLap) return 0;
    u16 avgLapTime = accTime / accLap;
    
    // lap duration in 1/10sec, lap length in meter, speed in 0.1 km/h
    // speed = 10 * (len / 1000) / (time / 36000)
    // speed = len * 360 / time
    u16 speed = (u32)eData.lapLength * 360 / avgLapTime;

    return speed;
}

/**
* ChronoGetLapNumber
*
* Parameters:
*       none
* Description:
*		Return number of lap since reset
* Returns:
*	    lap number
*/
u8    ChronoGetLapNumber(void)
{
    return accLap;
}
