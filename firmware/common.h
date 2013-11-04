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

#include <stdint.h>

#define u8      uint8_t
#define u16     uint16_t
#define u32     uint32_t
#define s8      int8_t
#define s16     int16_t
#define s32     int32_t

#define VERSION_SOFT_MAJOR       0
#define VERSION_SOFT_MINOR       1
#define VERSION_HARD             1

#define LOG(msg)  if(Flags.debug) printf(msg)

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
 * \struct Switch_t
 * \brief Store switch timings
 *
 * This structure is used for ignition and injection
 */
typedef struct {
   u16 activeUs;         /*!< active time switch in us */
   u16 activeDeg;        /*!< active time switch in deg */
   u16 inactiveUs;       /*!< inactive time switch in us */
   u16 inactiveDeg;      /*!< inactive time switch in deg */
}Switch_t;


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
    u8         injTable[12][12];  /*!< table for injection */
    u8         igniTable[12][12];  /*!< table for ignition */
}eeprom_data_t;

/**
 * \struct Current_Data_t
 * \brief Store current parameters of the system
 *
 * Current_data stores variable of system
 */
typedef struct {
   u16 battery;          /*!< in 10mV */
   u16 temp1;            /*!< in degC */
   u16 temp2;            /*!< in degC */
   u8 throttle;          /*!< in %    */
   u8 flybackFB;         /*!< in V    */
   u16 pressure;         /*!< in MPa  */
   u8  HVvalue;          /*!< PWM duty cycle for High voltage supply */
   u16 RPM;              /*!< RPM */
   u16 speed;            /*!< speed */
   u8 state;             /*!< state => 0 : stop, 1 : cranking, 2 : running, 3 : alarm, 4 : error 5 : stalled */
   Switch_t Ignition;         /*!< ignition mesurements */
   Switch_t Injection;        /*!< injection mesurements */
}Current_Data_t;


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
