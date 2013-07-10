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
   uint16_t activeUs;         /*!< active time switch in us */
   uint16_t activeDeg;        /*!< active time switch in deg */
   uint16_t inactiveUs;       /*!< inactive time switch in us */
   uint16_t inactiveDeg;      /*!< inactive time switch in deg */
}Switch_t;


/**
* \struct eeprom_data_t
* \brief Map of EEPROM memory
*
* eeprom_data consists off the EEPROM organisation
*/
typedef struct {
    unsigned char   ratio[6];      /*!< ratios to adjust ADC/DAC conversion in % */
    uint16_t        timerLed;      /*!< ratios to adjust ADC/DAC conversion in % */
    uint8_t         HVstep;        /*!< step of high voltage loop in %. 0 set manual mode */
    uint8_t         HVmanual;      /*!< HT duty cycle in manual mode in % */
}eeprom_data_t;

/**
 * \struct Current_Data_t
 * \brief Store current parameters of the system
 *
 * Current_data stores variable of system
 */
typedef struct {
   uint16_t battery;          /*!< in 10mV */
   uint16_t temp1;            /*!< in degC */
   uint16_t temp2;            /*!< in degC */
   uint8_t throttle;          /*!< in %    */
   uint8_t flybackFB;         /*!< in V    */
   uint16_t pressure;         /*!< in MPa  */
   uint8_t  HVvalue;          /*!< PWM duty cycle for High voltage supply */
   uint16_t RPM;              /*!< RPM */
   uint16_t speed;            /*!< speed */
   uint8_t state;             /*!< state => 0 : stop, 1 : cranking, 2 : running, 3 : alarm, 4 : error 5 : stalled */
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
   uint8_t newCmd;    	/*!< New command in uart buffer */
   uint8_t debug;      /*!< debug printf enable */
   uint8_t PMHEnable;  /*!< PMH enable */
   STATE PMHState;   /*!< PMH current state : active/inactive */
}Flags_t;



#endif
