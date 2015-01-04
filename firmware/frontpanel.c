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
 * \file frontpanel.c
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
 * This code is largely inspired by Adafruit library https://github.com/adafruit/Adafruit-RGB-LCD-Shield-Library
 * rewritten for pure C99 without Arduino libs dependancies
 */

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <ctype.h>
#include "frontpanel.h"
#include "common.h"


/*** System variables ***/
extern eeprom_data_t    eData;
extern Current_Data_t   gState;
extern intState_t       intState;

/*** Internal variables ***/
typedef enum{
    M_INIT,
    M_DEBUG, // force msg display
    M_NORMAL,
    M_AUTODIAG
    M_INJ_OFFSET,
    M_IGN_OFFSET,
    M_ASSERT,
    M_QTY
static menu_e           menuState;

static char             LCDBuffer[32];

/*** Internal functions ***/
static void MenuInit(void);
static menu_e MenuUpdate(u8 btn);

static void LCDInit(void);
static void LCDUpdate(void);

static void IOExpanderInit(void);
static u8 i2cStart(u8 address);
static void i2cStop(void);
static u8 i2cWrite(u8 data);
static u8 i2cReadAck(void);
static u8 i2cReadNack(void);

static u8 BtnRead(void);


/**
*
* FPInit
*
* Parameters:
*		type : HW type. To be set to 0, only LCD/buttons on I2C is supported. 
*
* Description:
*		Init of the front panel, LCD, menu...
*
* Returns:
*		none
*
*/
void FPInit(const u8 type)
{
    ASSERT(type == 0); // only Adafruit I2C supported for now

    /* Init I2C bus and MCP23017 IO expander */

    /* Init LCD */
    LCDInit();

    /* Init menu navigation */
    MenuInit();
    
    return;
}

/**
*
* FPRun
*
* Parameters:
*		None 
*
* Description:
*		Front panel run loop : to be called often to refresh LCD/read buttons.
*
* Returns:
*		none
*
*/
void FPRun(void)
{
    if(menuState == M_DEBUG) return;
    
    /* Read buttons */
    u8 btn = BtnRead();
   
    /* Update menu */
    MenuUpdate(btn);

    /* Update display */
    LCDUpdate();
    return;
}

/**
*
* FPSetLed
*
* Parameters:
*		color : led color 
*
* Description:
*		Set the backlight led to a given color
*
* Returns:
*		none
*
*/
void FPSetLed(const color_e color)
{
    return;
}

/**
*
* FPDebugMsg
*
* Parameters:
*		msg : message to display 
*
* Description:
*		Debug : force to display a given message. If null, back to normal mode.
*
* Returns:
*		none
*
*/
void FPDebugMsg(char *msg)
{
    if(!msg)
    {
        menuState = M_NORMAL;
        return;
    }

    menuState = M_DEBUG;
    strncpy(LCDBuffer, msg, 32);
    /* Force LCD update */
    LCDUpdate();

    return;
}

/********************** MENU MANAGEMENT ***************************/
static void MenuInit(void)
{
    menuState = M_INIT;
    memset(LCDBuffer, 0, 32);

    /* Display version */
    return;
}



/********************** LCD MANAGEMENT ***************************/




/********************** MCP23017 IO EXPANDER MANAGEMENT ***************************/

#define SCL_CLOCK  100000L
#define MCP23017_ADDRESS 0x0

// registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14


#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

static u8 i2cStart(u8 address)
{
    u8 status;
	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));
	// check value of TWI Status Register. Mask prescaler bits.
	status = TW_STATUS & 0xF8;
	if ((status != TW_START) && (status != TW_REP_START)) return 1;
	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));
	// check value of TWI Status Register. Mask prescaler bits.
	status = TW_STATUS & 0xF8;
	if ((status != TW_MT_SLA_ACK) && (status != TW_MR_SLA_ACK) ) return 1;
	return 0;
}

static void i2cStop(void)
{
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));
}

static u8 i2cWrite(u8 data)
{
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));
	// check value of TWI Status Register. Mask prescaler bits
	if((TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1;
	return 0;
}

static u8 i2cReadAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    
    return TWDR;
}

static u8 i2cReadNack(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}

static void IOExpanderInit(void)
{
    TWSR = 0;
    TWBR = ((F_CPU/SCL_CLOCK)-16)/2;
}

