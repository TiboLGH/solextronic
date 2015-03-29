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
 * \file frontpanel.h
 * \brief Manage front panel including LCD and input buttons.
 * \author Thibault Bouttevin
 * \date December 2014
 *
 * This file implements the management of the front panel based on the following stack:
 *  - HW : Adafruit RGB LCD Shield Kit (LCD + buttons driven by I2C)
 *  - I2C driver
 *  - LCD driver and buttons driver (mainly inspired from Adafruit Arduino lib)
 *  - Menu and logic display management
 *
 */


#ifndef FRONTPANEL_H
#define FRONTPANEL_H

#include <stdint.h>
#include "common.h"



typedef enum
{
    LED_OFF = 0,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    VIOLET,
    TEAL,
    WHITE
}color_e;

#define BUTTON_UP 0x08
#define BUTTON_DOWN 0x04
#define BUTTON_LEFT 0x10
#define BUTTON_RIGHT 0x02
#define BUTTON_SELECT 0x01


/* Public functions */
void FPInit(const u8 type);
void FPRun(void);
void FPSetLed(const color_e color);
void FPDebugMsg(char *msg);

#endif // COMMAND_H
