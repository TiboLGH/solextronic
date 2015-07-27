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
 * \file varDef.h
 * \brief Includes common declarations and definition of the project
 * \author Thibault Bouttevin
 * \date Dec 2013
 *
 * This file includes definitions of data structures used through the firmware, the tuner
 * software configuration file and the simulation.
 * 
 * Warning 1 ! this file will be used on both the microcontroller and the simulation test
 * program : use only c99 code, nothing specific to a platform and be careful about alignment !
 *
 * Warning 2 ! Comments are used to automagically derive documentation and tuner
 * software configuration file. Make sure to respect syntax !
 */

#ifndef VARDEF_H
#define VARDEF_H

#include <stdint.h>

#define u8      uint8_t
#define u16     uint16_t
#define u32     uint32_t
#define s8      int8_t
#define s16     int16_t
#define s32     int32_t

#define TABSIZE 10

/**
* \struct eeprom_data_t
* \brief Map of EEPROM memory
*
* eeprom_data consists off the EEPROM organisation
* Comments gives the format used for the tunerStudio configuration file
* = class, type, offset, shape, units, scale, translate, lo, hi, digits ; comments
*/
typedef struct __attribute__ ((__packed__)){ // packed for alignment when used in simulation
    u8         wheelImpulse;  /* scalar, U08,  xx,        " ",    1.00000,   0.00000,   0,     5,  0 ;  number of impulsion for a wheel rotation */
    u16        timerLed;      /* scalar, U16,  xx,       "ms",    1.00000,   0.00000,   0, 65535,  0 ;  internal timer */
    u8         wheelSize;     /* scalar, U08,  xx,        "m",    0.01000,   0.00000, 1.0,  2.55,  2 ;  distance run in one wheel rotation */
    u8         PMHOffset;     /* scalar, U08,  xx,      "deg",    1.00000,   0.00000,   0,   255,  0 ;  PMH offset in deg */
    u16        maxRPM;        /* scalar, U16,  xx,      "RPM",    1.00000,   0.00000,   0, 16000,  0 ;  RPM limitation, 0 if no limit */
    u8         maxTemp;       /* scalar, U08,  xx,     "degC",    1.00000,   0.00000,   0,   200,  0 ;  overheating threshold, 0 if no limit */
    u8         minBat;        /* scalar, U08,  xx,        "v",    0.10000,   0.00000, 7.0,  12.0,  1 ;  alarm on low battery */
    u8         minTps;        /* scalar, U08,  xx,         "",    1.00000,   0.00000,   0,   255,  0 ;  minimum value of TPS ADC */
    u8         maxTps;        /* scalar, U08,  xx,         "",    1.00000,   0.00000,   0,   255,  0 ;  maximum value of TPS ADC */
    u8         battRatio;     /* scalar, U08,  xx,         "",    1.00000,   0.00000,   0,   255,  0 ;  ratio for the battery ADC in 1/256 */
    u16        ignDuration;   /* scalar, U16,  xx,       "us",    1.00000,   0.00000,   0,  1000,  0 ;  ignition duration */
    u8         ignOverheat;   /* scalar, U08,  xx,      "deg",    1.00000,   0.00000,   0,    20,  0 ;  advance decrease in case of overheating */
    u8         noSparkAtDec;  /* bits,   U08,  xx,  [0:0], "spark at dec", "no spark at dec"         ;  1 to stop ignition when deceleration */
    u16        injOpen;       /* scalar, U16,  xx,       "us",    1.00000,   0.00000,  200,  2000, 0 ;  time to open injector */
    u16        injStart;      /* scalar, U16,  xx,       "us",    1.00000,   0.00000,  200,  2000, 0 ;  injector opening duration */
    u8         holdPWM;       /* scalar, U08,  xx,        "%",    1.00000,   0.00000,   10,   100, 0 ;  PWM ratio during hold time */
    u16        injRate;       /* scalar, U16,  xx,    "g/min",    1.00000,   0.00000,   50,   500, 0 ;  flow rate of injector */
    u8         injAdv;        /* scalar, U08,  xx,      "deg",    1.00000,   0.00000,    0,   255, 0 ;  mean injection advance */
    u8         targetAfr;     /* scalar, U08,  xx,         "",    0.10000,   0.00000,  7.0,  20.0, 1 ;  target Air Fuel ratio */
    u16        starterInj;    /* scalar, U16,  xx,       "us",    1.00000,   0.00000,  200,  5000, 0 ;  injection duration during crancking */
    u8         starterAdv;    /* scalar, U08,  xx,      "deg",    1.00000,   0.00000,   0,   255,  0 ;  advance during crancking */
    u8         injOverheat;   /* scalar, U08,  xx,        "%",    1.00000,   0.00000,    0,    50, 0 ;  injection increase in case of overheating */
    u8         noInjAtDec;    /* bits,   U08,  xx,  [0:0], "inj at dec", "no inj at dec"             ;  1 to stop injection when deceleration */
    u8         ignPolarity;   /* bits,   U08,  xx,  [0:0], "Active on low", "Active on high"         ;  0 active at low state */
    u8         injPolarity;   /* bits,   U08,  xx,  [0:0], "Active on low", "Active on high"         ;  0 active at low state */
    u8         pmhPolarity;   /* bits,   U08,  xx,  [0:0], "Active on low", "Active on high"         ;  0 active at low state */
    u8         pumpPolarity;  /* bits,   U08,  xx,  [0:0], "Active on low", "Active on high"         ;  0 active at low state */
    u16        injTestPW;     /* scalar, U16,  xx,       "us",    1.00000,   0.00000,  200, 10000, 0 ;  pulse width of injector test mode */
    u16        injTestCycles; /* scalar, U16,  xx,         "",    1.00000,   0.00000,    0,  1000, 0 ;  number of cycle of injector test mode. 0 to stop */
    u8         ignTestMode;   /* bits,   U08,  xx,  [0:0], "disable test mode", "Enable test mode"   ;  Enable/disable of ignition test mode */
    u16        rpmBins[TABSIZE]; /* array,  U16,  xx, [10],"RPM", 1.00000,   0.00000,    0, 10000, 0 ;  table of RPM indexes */
    u8         loadBins[TABSIZE]; /* array,  U08,  xx, [10],  "%", 1.00000,  0.00000,    0,   100, 0 ;  table of load/MAF indexes */
    u8         injTable[TABSIZE][TABSIZE];   /* array,  U08,  xx,[10x10],   "%",    1.00000,   0.00000,  10,   130,  0 ;  table for injection */
    u8         ignTable[TABSIZE][TABSIZE];   /* array,  U08,  xx,[10x10], "deg",    1.00000,   0.00000,   0,   200,  0 ;  table for ignition */
    u8         iatCal[TABSIZE][2]; /* array,  U08,   xx,  [10x2],  "deg",    1.00000,   0.00000,   0,   255,  0 ;  conversion table for IAT sensors */
    u8         cltCal[TABSIZE][2]; /* array,  U08,   xx,  [10x2],  "deg",    1.00000,   0.00000,   0,   255,  0 ;  conversion table for CLT sensors */
    u8         mapCal[TABSIZE][2]; /* array,  U08,   xx,  [10x2],  "kpa",    1.00000,   0.00000,   0,   255,  0 ;  conversion table for MAP sensor */
    u16        lapLength;     /* scalar, U16,  xx,        "m",    1.00000,   0.00000,    1, 65535, 0 ;  lap length in meter */
    u16        user1;         /* scalar, U16,  xx,         "",    1.00000,   0.00000,    0,  1000, 0 ;  for debug */
    u16        user2;         /* scalar, U16,  xx,         "",    1.00000,   0.00000,    0,  1000, 0 ;  for debug */ 
    u16        user3;         /* scalar, U16,  xx,         "",    1.00000,   0.00000,    0,  1000, 0 ;  for debug */
}eeprom_data_t;

/**
 * \struct Current_Data_t
 * \brief Store current parameters of the system
 *
 * Current_data stores variable of system
 * Comments gives the format used for the tunerStudio configuration file
 * type, size, offset, unit, scaling factor, scaling offset
 */
typedef struct  __attribute__ ((__packed__)){ // packed for alignment when used in simulation
   u8  rawAdc[5];        /* array,      U08,   xx, [5], " ",    1.000,  0.0 ; raw results of conversion, for calibration */
   u8  battery;          /* scalar,     U08,   xx,      "v",    0.100,  0.0 ; battery voltage */
   u8  CLT;              /* scalar,     U08,   xx,    "deg",    1.000,  0.0 ; motor temperature */
   u8  IAT;              /* scalar,     U08,   xx,    "deg",    1.000,  0.0 ; air temperature */
   u8  TPS;              /* scalar,     U08,   xx,      "%",    1.000,  0.0 ; throttle open ratio */
   u8  MAP;              /* scalar,     U08,   xx,    "kPa",    1.000,  0.0 ; manifold pressure */
   s8  TPSVariation;     /* scalar,     S08,   xx, "%/10ms",    1.000,  0.0 ; TPS variation speed */
   u16 rpm;              /* scalar,     U16,   xx,    "RPM",    1.000,  0.0 ; engine speed */
   u16 speed;            /* scalar,     U16,   xx,   "km/h",    1.000,  0.0 ; solex speed */
   u8  engineState;      /* scalar,     U08,   xx,    "bit",    1.000,  0.0 ; engine state */
                         /* cranking         = bits,   U08,   xx, [0:0] */
                         /* running          = bits,   U08,   xx, [1:1] */
                         /* overheat         = bits,   U08,   xx, [2:2] */
                         /* error            = bits,   U08,   xx, [3:3] */
                         /* stalled          = bits,   U08,   xx, [4:4] */
                         /* test_ign         = bits,   U08,   xx, [5:5] */
                         /* test_inj         = bits,   U08,   xx, [6:6] */
   u8  TPSState;         /* scalar,     U08,   xx,    "bit",    1.000,  0.0 ; TPS state */
                         /* idle             = bits,   U08,   xx, [0:0] */
                         /* wot              = bits,   U08,   xx, [1:1] */
                         /* opening          = bits,   U08,   xx, [2:2] */
                         /* closing          = bits,   U08,   xx, [3:3] */
   u16 injPulseWidth;    /* scalar,     U16,  xx,  "us",    1.000,  0.0 ; Injector active time */
   u16 injStart;         /* scalar,     U16,  xx, "deg",    1.000,  0.0 ; Injector start time before PMH */
   u8  advance;          /* scalar,     U08,  xx, "deg",    1.000,  0.0 ; Spark advance in deg before PMH */
   u16 second;           /* scalar,     U16,  xx, "sec",    1.000,  0.0 ; Current time in sec : will be set only when asked by UART command */
   u8  injTestMode;      /* scalar,     U08,  xx,   " ",    1.000,  0.0 ; current injector test mode */
   u16 injTestCycles;    /* scalar,     U16,  xx,   " ",    1.000,  0.0 ; current injector test cycles */
   u8  load;             /* scalar,     U08,  xx,  "%",     1.000,  0.0 ; current load */
   s8  ignOffset;        /* scalar,     S08,  xx,  "deg",   1.000,  0.0 ; ignition offset adjustable runtime */
   s16 injOffset;        /* scalar,     S16,  xx,  "us",    1.000,  0.0 ; injection duration offset adjustable runtime */
   s8  injStartOffset;   /* scalar,     S08,  xx,  "deg",   1.000,  0.0 ; injection start offset adjustable runtime */
   u16 debug1;           /* scalar,     U16,  xx,  "%",     1.000,  0.0 ; debug for user */
   u16 debug2;           /* scalar,     U16,  xx,  "%",     1.000,  0.0 ; debug for user */
   u16 debug3;           /* scalar,     U16,  xx,  "%",     1.000,  0.0 ; debug for user */
}Current_Data_t;
#endif
