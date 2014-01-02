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
 * Warning ! this file wille be used on both the microcontroller and the simulation test
 * program : use only c99 code, nothing specific to a platform !
 *
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

/**
* \struct eeprom_data_t
* \brief Map of EEPROM memory
*
* eeprom_data consists off the EEPROM organisation
* Comments gives the format used for the tunerStudio configuration file
* = class, type, offset, shape, units, scale, translate, lo, hi, digits ; comments
*/
typedef struct {
    u8         ratio[5];      /*!< = array,  U08,   0,  [5],  "%",    1.00000,   0.00000,   0,   200,  0 ;  ratios to adjust ADC/DAC conversion */
    u8         wheelImpulse;  /*!< = scalar, U08,   5,        " ",    1.00000,   0.00000,   0,     5,  0 ;  number of impulsion for a wheel rotation */
    u16        timerLed;      /*!< = scalar, U16,   6,       "ms",    1.00000,   0.00000,   0, 65535,  0 ;  internal timer */
    u8         HVstep;        /*!< = scalar, U08,   8,     "step",    1.00000,   0.00000,   0,    50,  0 ;  step of high voltage loop in %. 0 set manual mode */
    u8         HVmanual;      /*!< = scalar, U08,   9,        "%",    1.00000,   0.00000,   0,   200,  0 ;  HT duty cycle in manual mode in % */
    u8         wheelSize;     /*!< = scalar, U08,  10,        "m",    0.01000,   0.00000, 1.0,  2.55,  2 ;  distance run in one wheel rotation */
    u8         PMHOffset;     /*!< = scalar, U08,  11,      "deg",    1.00000,   0.00000,   0,   255,  0 ;  PMH offset in deg */
    u16        maxRPM;        /*!< = scalar, U16,  12,      "RPM",    1.00000,   0.00000,   0, 16000,  0 ;  RPM limitation, 0 if no limit */
    u16        maxTemp;       /*!< = scalar, U16,  14,     "degC",    1.00000,   0.00000,   0,   200,  0 ;  overheating threshold, 0 if no limit */
    u8         minBat;        /*!< = scalar, U08,  16,        "v",    0.10000,   0.00000, 7.0,  12.0,  1 ;  alarm on low battery */
    u16        igniDuration;  /*!< = scalar, U16,  17,       "us",    1.00000,   0.00000,   0,  1000,  0 ;  ignition duration */
    u8         starterAdv;    /*!< = scalar, U08,  19,      "deg",    1.00000,   0.00000,   0,   255,  0 ;  advance during crancking */
    u8         igniOverheat;  /*!< = scalar, U08,  20,      "deg",    1.00000,   0.00000,   0,    20,  0 ;  advance decrease in case of overheating */
    u8         noSparkAtDec;  /*!< = bits,   U08,  21,  [0:0], "spark at dec", "no spark at dec"         ;  1 to stop ignition when deceleration */
    u16        injOpen;       /*!< = scalar, U16,  22,       "us",    1.00000,   0.00000,  200,  2000, 0 ;  time to open injector */
    u16        injRate;       /*!< = scalar, U16,  24,    "g/sec",    1.00000,   0.00000,   50,   500, 0 ;  flow rate of injector */
    u8         injAdv;        /*!< = scalar, U08,  26,      "deg",    1.00000,   0.00000,    0,   255, 0 ;  mean injection advance */
    u16        starterInj;    /*!< = scalar, U16,  27,       "us",    1.00000,   0.00000,  200,  5000, 0 ;  injection duration during crancking */
    u8         injOverheat;   /*!< = scalar, U08,  29,        "%",    1.00000,   0.00000,    0,    50, 0 ;  injection increase in case of overheating */
    u16        injFullOpen;   /*!< = scalar, U16,  31,       "us",    1.00000,   0.00000,  500,  5000, 0 ;  injection duration at full throttle */
    u8         noInjAtDec;    /*!< = bits,   U08,  33,  [0:0], "inj at dec", "no inj at dec"             ;  1 to stop injection when deceleration */
    u16        injStart;      /*!< = scalar, U16,  34,       "us",    1.00000,   0.00000,  200,  2000, 0 ;  injector opening duration */
    u8         holdPWM;       /*!< = scalar, U08,  36,        "%",    1.00000,   0.00000,   10,   100, 0 ;  PWM ratio during hold time */
    u8         igniPolarity;  /*!< = bits,   U08,  37, "Active on low", "Active on high"                 ;  0 active at low state */
    u8         injPolarity;   /*!< = bits,   U08,  38, "Active on low", "Active on high"                 ;  0 active at low state */
    u8         pmhPolarity;   /*!< = bits,   U08,  39, "Active on low", "Active on high"                 ;  0 active at low state */
    u8         pumpPolarity;  /*!< = bits,   U08,  40, "Active on low", "Active on high"                 ;  0 active at low state */
    u16        injTestPW;     /*!< = scalar, U16,  41,       "us",    1.00000,   0.00000,  200, 10000, 0 ;  pulse width of injector test mode */
    u16        injTestCycles; /*!< = scalar, U16,  43,         "",    1.00000,   0.00000,    0,  1000, 0 ;  number of cycle of injector test mode. 0 to stop */
    u16        rpmBins[10];   /*!< = array,  U16,  45, [10],"RPM",    1.00000,   0.00000,    0, 10000, 0 ;  table of RPM indexes */
    u8         loadBins[10];  /*!< = array,  U08,  65, [10],  "%",    1.00000,   0.00000,    0,   100, 0 ;  table of load/MAF indexes */
    u8         injTable[1][1];   /*!< = array,  U08,  75,[10x10],   "%",    1.00000,   0.00000,  10,   130,  0 ;  table for injection */
    u8         igniTable[1][1];  /*!< = array,  U08, 175,[10x10], "deg",    1.00000,   0.00000,   0,   200,  0 ;  table for ignition */
    u16        user1;         /*!< = scalar, U16, 275,         "",    1.00000,   0.00000,    0,  1000, 0 ;  for debug */
    u16        user2;         /*!< = scalar, U16, 277,         "",    1.00000,   0.00000,    0,  1000, 0 ;  for debug */
    u16        user3;         /*!< = scalar, U16, 279,         "",    1.00000,   0.00000,    0,  1000, 0 ;  for debug */
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
   u8  battery;          /*!< = scalar,     U08,    0,   "v",    0.100,  0.0 */
   u8  tempMotor;        /*!< = scalar,     U08,    1,  "°C",    1.000,  0.0 */
   u8  tempAir;          /*!< = scalar,     U08,    2,  "°C",    1.000,  0.0 */
   u8  throttle;         /*!< = scalar,     U08,    3,   "%",    1.000,  0.0 */
   u16 pressure;         /*!< = scalar,     U16,    4, "kPa",    1.000,  0.0 */
   u8  HVvalue;          /*!< = scalar,     U08,    6,   "%",    1.000,  0.0. PWM duty cycle for High voltage supply */
   u16 rpm;              /*!< = scalar,     U16,    7, "RPM",    1.000,  0.0 */
   u16 speed;            /*!< = scalar,     U16,    9,"km/h",    1.000,  0.0 */
   u8  engine;           /*!< = scalar,     U08,   10,"bits",    1.000,  0.0 */
                         /*!< Engine status - bit field for engine */
                         /*!< ready:    bit 0 => 0 = engine not ready 1 = ready to run */
                         /*!< crank:    bit 1 => 0 = engine not cranking 1 = engine cranking */
                         /*!< startw:   bit 2 => 0 = not in startup warmup 1 = in warmup enrichment */
                         /*!< warmup:   bit 3 => 0 = not in warmup 1 = in warmup */
                         /*!< tpsaen:   bit 4 => 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
                         /*!< tpsden:   bit 5 => 0 = not in deacceleration mode 1 = in deacceleration mode */
                         /*!< revlim:   bit 6 => 0 = not in rev limiter mode 1 = rev limiter mode*/
                         /*!< overheat: bit 7 => 0 = not in overheat mode 1 = overheat mode*/
                         /*!< ready            = bits,   U08,   10, [0:0] */
                         /*!< crank            = bits,   U08,   10, [1:1] */
                         /*!< startw           = bits,   U08,   10, [2:2] */
                         /*!< warmup           = bits,   U08,   10, [3:3] */
                         /*!< tpsaen           = bits,   U08,   10, [4:4] */
                         /*!< tpsden           = bits,   U08,   10, [5:5] */ 
                         /*!< revlim           = bits,   U08,   10, [6:6] */
                         /*!< overheat         = bits,   U08,   10, [7:7] */
   u16 injPulseWidth;    /*!< = scalar,     U16,   11,  "us",    1.000,  0.0. Injector active time */
   u16 injStart;         /*!< = scalar,     U16,   13, "deg",    1.000,  0.0. Injector start time before PMH */
   u8  advance;          /*!< = scalar,     U08,   15, "deg",    1.000,  0.0. Spark advance in deg before PMH */
   u16 second;           /*!< = scalar,     U16,   17, "sec",    1.000,  0.0. Current time in sec : will be set only when asked by UART command */
   u16 debug1;           /*!< = scalar,     U16,   19,  "%",     1.000,  0.0 */
   u16 debug2;           /*!< = scalar,     U16,   21,  "%",     1.000,  0.0 */
   u16 debug3;           /*!< = scalar,     U16,   23,  "%",     1.000,  0.0 */
   u8 kpaix;             /*!< = scalar,     U08,   25,  "%",     1.000,  0.0 */
}Current_Data_t;
#endif
