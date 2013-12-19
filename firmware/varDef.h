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
*/
typedef struct {
    u8         ratio[5];      /*!< ratios to adjust ADC/DAC conversion in % */
    u16        timerLed;      /*!< ratios to adjust ADC/DAC conversion in % */
    u8         HVstep;        /*!< step of high voltage loop in %. 0 set manual mode */
    u8         HVmanual;      /*!< HT duty cycle in manual mode in % */
    u8         wheelSize;     /*!< distance run in one wheel rotation */
    u8         PMHOffset;     /*!< PMH offset in deg */
    u16        maxRPM;        /*!< RPM limitation, 0 if no limit */
    u8         maxTemp;       /*!< overheating threshold, 0 if no limit */
    u8         minBat;        /*!< alarm on low battery, 0.1v */
    u16        igniDuration;  /*!< ignition duration in us */
    u8         starterAdv;    /*!< advance during crancking in deg */
    u8         igniOverheat;  /*!< advance decrease in case of overheating in deg */
    u8         noSparkAtDec;  /*!< 1 to stop ignition when deceleration */
    u16        injOpen;       /*!< time to open injector in us */
    u16        injRate;       /*!< flow rate of injector in g/us */
    u8         injAdv;        /*!< mean injection advance in deg */
    u16        starterInj;    /*!< injection duration during crancking in us */
    u16        injOverheat;   /*!< injection increase in case of overheating in % */
    u16        injFullOpen;   /*!< injection duration at full throttle in us */
    u8         noInjAtDec;    /*!< 1 to stop injection when deceleration */
    u16        injStart;      /*!< injector opening duration in us */
    u8         holdPWM;       /*!< PWM ratio during hold time in % */
    u8         igniPolarity;  /*!< 0 active at low state */
    u8         injPolarity;   /*!< 0 active at low state */
    u8         pmhPolarity;   /*!< 0 active at low state */
    u8         pumpPolarity;  /*!< 0 active at low state */
    u16        injTestPW;     /*!< pulse width of injector test mode in us. Max 10ms */
    u16        injTestCycles; /*!< number of cycle of injector test mode. */
    u16        rpmBins[10];   /*!< table of RPM indexes */
    u8         loadBins[10];  /*!< table of load/MAF indexes */
    u8         injTable[10][10];  /*!< table for injection */
    u8         igniTable[10][10];  /*!< table for ignition */
    u8         user1;
    u8         user2;
    u8         user3;
}eeprom_data_t;

/**
 * \struct Current_Data_t
 * \brief Store current parameters of the system
 *
 * Current_data stores variable of system
 */
typedef struct {
   u8  battery;          /*!< in 100mV */
   u8  temp1;            /*!< in degC */
   u8  temp2;            /*!< in degC */
   u8  throttle;         /*!< in %    */
   u16 pressure;         /*!< in kPa  */
   u8  HVvalue;          /*!< PWM duty cycle for High voltage supply */
   u16 RPM;              /*!< RPM */
   u16 speed;            /*!< speed */
   u8  engine;           /*!< state => 0 : stop, 1 : cranking, 2 : running, 3 : alarm, 4 : error 5 : stalled */
   u16 injPulseWidth;    /*!< injector on time in us */
   u16 injStart;         /*!< injector start time in deg before PMH */
   u8  advance;          /*!< spark advance in deg before PMH */
   u16 time;             /*!< current time in sec : will be set only when asked by UART command */
   u16 debug1;
   u16 debug2;
   u16 debug3;
}Current_Data_t;

#endif
