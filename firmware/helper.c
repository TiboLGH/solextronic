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
//#define V(msg, ...) fprintf(stdout, msg, ##__VA_ARGS__ )

u16 Interp2D(u8 *table, u16 rpm, u8 load)
{
    u8 rpmIdx, loadIdx;
  
    /****** 1. Step 1 : locate surronding values in table  *******/
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

    /****** 2.2. First interpolate along rpm axis  *******/
    if(!loadDelta)
    {
        return valueLow;
    }else{
        delta = valueHigh - valueLow;
        return (u16)(valueLow + ((load - eData.loadBins[loadIdx]) * (long)delta) / loadDelta);
    }
}


