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
 * \file helper.c
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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "varDef.h"

extern eeprom_data_t    eData;
#ifdef SIM_HOST
#define V(msg, ...) fprintf(stdout, msg, ##__VA_ARGS__ )
#else
#define V(msg, ...) do{}while(0);
#endif

u16 Interp2D(volatile u8 *table, u16 rpm, u8 load)
{
    u8 rpmIdx, loadIdx;
  
    /****** 1. Step 1 : locate surrounding values in table  *******/
    /****** 1.1. RPM coordinate lookup *******/
    // Check for limit
    if(rpm > eData.rpmBins[TABSIZE-1])
    {
        // force to max value, no extrapolation 
        rpm = eData.rpmBins[TABSIZE-1];
        rpmIdx = TABSIZE-1;
    }
    else if(rpm < eData.rpmBins[0])
    {
        rpm = eData.rpmBins[0];
        rpmIdx = 0;
    }
    else // in the table
    {
        for(rpmIdx=0; rpmIdx<TABSIZE-1; ++rpmIdx)
        {
            if (rpm >= eData.rpmBins[rpmIdx] && rpm <= eData.rpmBins[rpmIdx+1])
            {
                break;
            }
        }
    }

    /****** 1.2. Load coordinate lookup *******/
    // Check for limit
    if(load > eData.loadBins[TABSIZE-1])
    {
        // force to max value, no extrapolation 
        load = eData.loadBins[TABSIZE-1];
        loadIdx = TABSIZE-1;
    }
    else if(load < eData.loadBins[0])
    {
        load = eData.loadBins[0];
        loadIdx = 0;
    }
    else // in the table
    {
        for(loadIdx=0; loadIdx<TABSIZE-1; ++loadIdx)
        {
            if (load >= eData.loadBins[loadIdx] && load <= eData.loadBins[loadIdx+1])
            {
                break;
            }
        }
    }
  
    /****** 2. Step 2 : interpolation between points of step 1  *******/
    u16 rpmDelta = eData.rpmBins[rpmIdx+1] - eData.rpmBins[rpmIdx];
    u16 loadDelta = eData.loadBins[loadIdx+1] - eData.loadBins[loadIdx];
    /****** 2.1. First interpolate along rpm axis  *******/
    u16 valueLow, valueHigh, delta;

    if(!rpmDelta)
    {
        valueLow  = table[rpmIdx * TABSIZE + loadIdx];
        valueHigh = table[rpmIdx * TABSIZE + loadIdx+1];
    }else{    
        // Compute value low = interpolation along rpm for lower load
        delta = table[(rpmIdx+1) * TABSIZE + loadIdx] - table[rpmIdx * TABSIZE + loadIdx];
        valueLow = table[rpmIdx * TABSIZE + loadIdx] + ((rpm - eData.rpmBins[rpmIdx]) * (long)delta) / rpmDelta;
        // Compute value high = interpolation along rpm for higher load
        delta = table[(rpmIdx+1) * TABSIZE + loadIdx+1] - table[rpmIdx * TABSIZE + loadIdx+1];
        valueHigh = table[rpmIdx * TABSIZE + loadIdx+1] + ((rpm - eData.rpmBins[rpmIdx]) * (long)delta) / rpmDelta;
    }

    /****** 2.2. Second interpolate along load axis  *******/
    if(!loadDelta)
    {
        return valueLow;
    }else{
        delta = valueHigh - valueLow;
        return (u16)(valueLow + ((load - eData.loadBins[loadIdx]) * (long)delta) / loadDelta);
    }
}

/* Interpolation over 1 dimension
 * Constraints (not checked by the function !)
 *  - x table (table[][0]) shall be sorted in rising values
 *  - delta between consecutive y values shall be in {-128,+127} 
 */ 
u8  Interp1D(volatile u8 table[TABSIZE][2], u8 adcVal)
{
    //for(int i = 0; i < TABSIZE; i++) V("%d temp %d : adc %d\n", i, table[i][0], table[i][1]);
    u8 i = 0;
    // Check for limit
    if(adcVal > table[TABSIZE-1][0])
    {
        // force to max value, no extrapolation 
        return table[TABSIZE-1][1];
    }
    else if(adcVal < table[0][0])
    {
        return table[0][1];
    }
    else // in the table : interpolate
    {
        for(i=0; i<TABSIZE-1; ++i)
        {
            if (adcVal < table[i][0]) break;
        }
        u8 deltaX = table[i][0] - table[i-1][0];
        if(!deltaX) return 0; // hum that's bad 
        s8 deltaY = table[i][1] - table[i-1][1];
        if(!deltaY) return table[i][1]; 
        return table[i-1][1] + ((s16)(adcVal - table[i-1][0]) * deltaY) / deltaX; 
    }
}

/* Interpolation over 2 points
 * Return y value corresponding to x value from (x1,y1)(x2,y2) points
 * Constraint : x1<=x2, y>0
 */ 
u16 Interp2points(u8 x, u8 x1, u8 x2, u16 y1, u16 y2)
{
    u8 deltaX = x2-x1;
    if(!deltaX) return 0; //hum that's bad
    s16 deltaY = y2-y1;
    if(!deltaY) return y1;
    return y1 + ((s16)(x - x1) * deltaY) / deltaX;
}

