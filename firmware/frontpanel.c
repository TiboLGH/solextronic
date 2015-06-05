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
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/twi.h>
#include <ctype.h>
#include "frontpanel.h"
#include "common.h"
#include "chrono.h"
#include "version.h"


/*** System variables ***/
extern eeprom_data_t    eData;
extern Current_Data_t   gState;
extern intState_t       intState;
extern const char       signature[];

/*** Internal variables ***/
typedef enum{
    M_INIT,
    M_DEBUG, // force msg display
    M_ASSERT,
    M_NORMAL, // default view
    M_INPUT,
    M_IGN_OFFSET,
    M_INJ_OFFSET,
    M_INJ_START_OFFSET,
    M_AUTODIAG,
	M_CHRONO,
    M_QTY
}menu_e;
static menu_e           menuState;

static char             lcdBuffer[33]; 	// 2 lines, 16 chars + 1 for terminal string

/*** Internal functions ***/
static void MenuInit(void);
static menu_e MenuUpdate(u8 btn);

static void LCDInit(void);
static void LCDUpdate(void);
static u8 BtnRead(void);

static void IOExpanderInit(void);
static void IOExpanderWriteLCDPin(u8 val);
static void IOExpanderWriteBackLightPins(u8 val);
static u8 IOExpanderReadBtn(void);

/**
*
* FPInit
*
* Parameters:
*		type : HW type. To be set to 0, only LCD/buttons on I2C is supported for now... 
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
	IOExpanderInit();

    /* Init LCD */
    LCDInit();
    FPSetLed(WHITE);

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
    IOExpanderWriteBackLightPins((u8) color);
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
    strncpy(lcdBuffer, msg, 33);
    /* Force LCD update */
    LCDUpdate();

    return;
}

/********************** MENU MANAGEMENT ***************************/
static void MenuInit(void)
{
    menuState = M_INIT;
    memset(lcdBuffer, 0, 33);

    /* Set backlight */
    FPSetLed(WHITE); 

    return;
}

#define NAV(button, state) if(btn & BUTTON_## button) {menuState = state;}

static menu_e MenuUpdate(u8 btn)
{
    static u8 inputUnit = 0;
    static u8 normalView1 = 0;
    static s16  var = 0;

    switch(menuState)
    {
        case M_INIT: // start. Display version
            snprintf_P(lcdBuffer, 33, PSTR("%s      "), signature);
            NAV(DOWN, M_NORMAL);
            break;

        case M_DEBUG: // force msg display
            snprintf_P(lcdBuffer, 33, PSTR("Debug TODO !"));
            if(btn) menuState = M_NORMAL;
            break;

        case M_ASSERT: //the only exit is reset !
            snprintf_P(lcdBuffer, 33, PSTR("Assert TODO !"));
            break; 

        case M_NORMAL: // default view
            // Display
            if(normalView1)
            {
                snprintf_P(lcdBuffer, 33, PSTR("%4drpm %2d.%1dkm/h%4du %3dd %3dd "), gState.rpm, gState.speed/10, gState.speed%10, gState.injPulseWidth, gState.CLT, gState.advance);
            }else{ //alternate view
                snprintf_P(lcdBuffer, 33, PSTR("%4drpm %2d.%1dkm/h%2d.%1dv %3d%% %2ddeg"), gState.rpm, gState.speed/10, gState.speed%10, gState.battery/10, gState.battery%10, gState.TPS, gState.IAT);
            }
            if((btn & BUTTON_PLUS) || (btn & BUTTON_MINUS)) normalView1 ^= 1;
            // chrono top lap on OK button
            if(btn & BUTTON_OK) ChronoTopLap();
            // Navigation
            NAV(DOWN, M_INPUT);
            NAV(UP, M_CHRONO);
            break;

        case M_INPUT:
            // Display
            if(inputUnit) //raw values
            {
                snprintf_P(lcdBuffer, 33, PSTR("B%3d C %3d I %3d T %3d  M %3d   "), gState.rawAdc[0], gState.rawAdc[1], gState.rawAdc[2], gState.rawAdc[3], gState.rawAdc[4]);
            }else{ // converted values
                snprintf_P(lcdBuffer, 33, PSTR("%2d.%1dv %3dd  %3dd  %3d%%  %3dkpa  "), gState.battery/10, gState.battery%10, gState.CLT, gState.IAT, gState.TPS, gState.MAP);
            }
            // Navigation
            if((btn & BUTTON_PLUS) || (btn & BUTTON_MINUS)) inputUnit ^= 1;
            NAV(DOWN, M_IGN_OFFSET);
            NAV(UP, M_NORMAL);
            break;

        case M_IGN_OFFSET:
            snprintf_P(lcdBuffer, 33, PSTR("Ign Offset %2ddeg+/-/OK          "), var);
            // Navigation
            if(btn & BUTTON_PLUS) var++;
            else if(btn & BUTTON_MINUS) var--;
            else if(btn & BUTTON_OK) gState.ignOffset = (s8)var;
            NAV(DOWN, M_INJ_OFFSET);
            NAV(UP, M_INPUT);
            break;

        case M_INJ_OFFSET:
            snprintf_P(lcdBuffer, 33, PSTR("Inj Offset  %3du+/-/OK          "), var);
            // Navigation
            if(btn & BUTTON_PLUS) var++;
            else if(btn & BUTTON_MINUS) var--;
            else if(btn & BUTTON_OK) gState.injOffset = var;
            NAV(DOWN, M_INJ_START_OFFSET);
            NAV(UP, M_IGN_OFFSET);
            break;

        case M_INJ_START_OFFSET:
            snprintf_P(lcdBuffer, 33, PSTR("Ign Offset %2ddeg+/-/OK          "), var);
            // Navigation
            if(btn & BUTTON_PLUS) var++;
            else if(btn & BUTTON_MINUS) var--;
            else if(btn & BUTTON_OK) gState.injStartOffset = (s8)var;
            NAV(DOWN, M_AUTODIAG);
            NAV(UP, M_INJ_OFFSET);
            break;

        case M_AUTODIAG:
            snprintf_P(lcdBuffer, 33, PSTR("Autodiag TODO !"));
            // Navigation
            NAV(DOWN, M_CHRONO);
            NAV(UP, M_INJ_START_OFFSET);
            break;

        case M_CHRONO:
            ChronoGetCurrentTime(lcdBuffer);
            ChronoGetAvgLapTime(lcdBuffer + 8);
            u16 speed = ChronoGetAvgSpeed();
            snprintf_P(lcdBuffer+16, 17, PSTR("%2d.%1dkm/h %2d tr"), speed/10, speed%10, ChronoGetLapNumber());
            // chrono reset on OK button
            if(btn & BUTTON_OK) ChronoReset();
            // Navigation
            NAV(DOWN, M_NORMAL);
            NAV(UP, M_AUTODIAG);
            break;

        default:
            menuState = M_NORMAL;
    }
    return menuState;
}

/********************** LCD MANAGEMENT ***************************/
// commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON      0x04
#define LCD_DISPLAYOFF     0x00
#define LCD_CURSORON       0x02
#define LCD_CURSOROFF      0x00
#define LCD_BLINKON        0x01
#define LCD_BLINKOFF       0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE    0x08
#define LCD_CURSORMOVE     0x00
#define LCD_MOVERIGHT      0x04
#define LCD_MOVELEFT       0x00

// flags for function set
#define LCD_4BITMODE       0x00
#define LCD_2LINE          0x08
#define LCD_5x8DOTS        0x00

#define LCD_DATA           0x01
#define LCD_CMD            0x00

//Pins assigment
#define RS 0x80
#define RW 0x40
#define EN 0x20
#define D4 0x10
#define D5 0x08
#define D6 0x04
#define D7 0x02

static void Send(u8 value, u8 mode, u8 lowNibbleOnly) 
{
    u8 out;
    if(!lowNibbleOnly)
    {
        //set RS
        out = EN;
        if(mode == LCD_DATA) out |= RS;
        // set high nibble
        // bit reverse and shift as pins are not is the natural order :-(
        if(value & (1 << 7)) out |= D7;
        if(value & (1 << 6)) out |= D6;
        if(value & (1 << 5)) out |= D5;
        if(value & (1 << 4)) out |= D4;
        IOExpanderWriteLCDPin(out);
        // pulse enable
        _delay_us(1);
        out &= ~(EN);
        IOExpanderWriteLCDPin(out);
        _delay_us(1);
    }

    // set low nibble
    out = EN;
    if(mode == LCD_DATA) out |= RS;
    if(value & (1 << 3)) out |= D7;
    if(value & (1 << 2)) out |= D6;
    if(value & (1 << 1)) out |= D5;
    if(value & (1 << 0)) out |= D4;
    IOExpanderWriteLCDPin(out);
    // pulse enable
    _delay_us(1);
    out &= ~(EN);
    IOExpanderWriteLCDPin(out);
    _delay_us(100);
}

static void LCDInit(void)
{
	memset(lcdBuffer, ' ', 32);

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    _delay_us(50000); 

    // init procedure
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    Send(0x03, LCD_CMD, 1);
    _delay_us(4500); // wait min 4.1ms
    // second try
    Send(0x03, LCD_CMD, 1);
    _delay_us(4500); // wait min 4.1ms
    // third go!
    Send(0x03, LCD_CMD, 1);
    _delay_us(150);
    // set to 4-bit interface
    Send(0x02, LCD_CMD, 1);
    // finally, set # lines, font size, etc.
    Send(LCD_FUNCTIONSET | LCD_2LINE | LCD_4BITMODE | LCD_5x8DOTS, LCD_CMD, 0);  
    // turn the display on with no cursor or blinking default
    Send(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF, LCD_CMD, 0);
    // set the entry mode
    Send(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT, LCD_CMD, 0);
    // clear
    Send(LCD_CLEARDISPLAY, LCD_CMD, 0);
}

static void LCDUpdate(void)
{
    // set address to 0
    Send(LCD_SETDDRAMADDR | 0x00, LCD_CMD, 0);
    _delay_us(50);
    // write the 32 chars : ~60µs per char => 2ms... a bit long, to be sliced
    // address is auto-incremented 
    for(u8 i = 0; i < 32; i++)
    {
        Send(lcdBuffer[i], LCD_DATA, 0);
        _delay_us(50);
        if(i == 15) // 2nd line
        {
            Send(LCD_SETDDRAMADDR | 0x40, LCD_CMD, 0);
            _delay_us(50);
        }
    }
}

static u8 BtnRead(void)
{
    return IOExpanderReadBtn();
}


/********************** MCP23017 IO EXPANDER MANAGEMENT ***************************/

#define SCL_CLOCK  100000L
#define MCP23017_ADDRESS 0x40

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

/*static u8 i2cReadAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    
    return TWDR;
}*/

static u8 i2cReadNack(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}


static void IOExpanderRegWrite(u8 reg, u8 val)
{
	if( reg > 22) return;

	i2cStart(MCP23017_ADDRESS);
	i2cWrite(reg);
	i2cWrite(val);
	i2cStop();
}

static u8 IOExpanderRegRead(u8 reg)
{
	if(reg > 22) return 0;

	i2cStart(MCP23017_ADDRESS);
	i2cWrite(reg);
	i2cStart(MCP23017_ADDRESS+1); //for read
	u8 read = i2cReadNack();
	i2cStop();
    return read;
}

static u8 _portB = 0;

static void IOExpanderWriteLCDPin(u8 val)
{
    _portB &= 0x1;
    _portB |= (val & 0xFE); 
	IOExpanderRegWrite(MCP23017_GPIOB, _portB);
}

static void IOExpanderWriteBackLightPins(u8 val)
{
	// shared between ports A & B :-(
    _portB &= 0xFE;
    _portB |= (~(val >> 2) & 0x1); 
	IOExpanderRegWrite(MCP23017_GPIOB, _portB);
	IOExpanderRegWrite(MCP23017_GPIOA, ~(val << 6));
}

static u8 IOExpanderReadBtn(void)
{
	return (0x1F & ~IOExpanderRegRead(MCP23017_GPIOA));
}

static void IOExpanderInit(void)
{
	// I2C controller initialisation
    TWSR = 0;
    TWBR = ((F_CPU/SCL_CLOCK)-16)/2;

	// Initialize the IO Expander
	// Pinout : A port (0->7), B port (8->15) 
	// RS pin 15
	// RW pin 14 (not used)
	// EN pin 13
	// D4 pin 12
	// D5 pin 11
	// D6 pin 10
	// D7 pin 9
	// LED 2 pin 8
	// LED 1 pin 7
	// LED 0 pin 6
	// BTN 4 pin 4
	// BTN 3 pin 3
	// BTN 2 pin 2
	// BTN 1 pin 1
	// BTN 0 pin 0
	
    // IO direction and pull up on buttons
	IOExpanderRegWrite(MCP23017_IODIRA, 0x1F); 
	IOExpanderRegWrite(MCP23017_IODIRB, 0x00); 
	IOExpanderRegWrite(MCP23017_GPPUA, 0x1F); 
	IOExpanderRegWrite(MCP23017_GPPUB, 0x00); 
}

