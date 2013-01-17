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
 * \file command.c
 * \brief Manage remote commands.
 * \author Thibault Bouttevin
 * \date October 2012
 *
 * This file implements actions triggered in response to remote commands received through RS232 interface.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <ctype.h>
#include "command.h"
#include "common.h"
#include "platform.h"


/*** Commands ***/



#define Cmd_SetLed          11

extern Flags_t          Flags;
extern eeprom_data_t    eData;
extern Current_Data_t   CurrentValues;


volatile uint8_t bufTx[BUFSIZE];
volatile uint8_t bufRx[BUFSIZE];
volatile uint8_t indexRx;
volatile uint8_t indexTxRead;
volatile uint8_t indexTxWrite;

static int32_t 	args[MAXARGS];
static commandType_t command;

uint8_t dbg[16];

uint8_t GetArg(uint8_t *argIndex, uint8_t *indexRxRead);
uint8_t SetArg(uint8_t argIndex);

/**
*
* ProcessCommand
*
* Parameters:
*		none
*
* Description:
*		read usart buffer and call command handler
*
* Returns:
*		none
*
*/
void ProcessCommand(void)
{
    state_e state = IDLE;
    uint8_t indexRxRead = 0;
    uint8_t argIndex = 0;
    uint8_t c;

    /* read reception buffer until last received char */
    while(indexRxRead != indexRx)
    {
        /* get character from reception buffer */
        c = bufRx[indexRxRead++];

        switch(state)
        {
            /* wait first valid character : r,v,s,g,d characters */
            case IDLE:
                if(c == 'r' || c == 'v')
                { 
                    state = WAIT_CR; // wait for CR
                }
                else if(c == 's' || c == 'g' || c == 'd')
                {
                    state = WAIT_ARG; // wait for args
                }
                command.cmdType = c;
                break;

            /* wait CR */
            case WAIT_CR:
                if(c == '\r')
                {
                    state = END;
                }else{ /* character is not valid, reset FSM */
                    state = IDLE;	
                }
                break;

            /* wait for arguments */
            case WAIT_ARG:
                c = GetArg(&argIndex, &indexRxRead);
                if(c == LAST_ARG)
                {
                    state = END;
                }
                else if(c == NEXT_ARG)
                {
                    state = WAIT_ARG;
                }
                else
                {
                    /* character is not valid, reset FSM */
                    state = IDLE;
                }
                break;
            case END:
                break;
        }// switch state
    }// while

    if(state == END)
    {
        command.nbArgSet = argIndex;
        command.nbArgGet = argIndex;
        CommandDispatch();
        state = 0;
    }
    indexRx = 0; 
    USART_RX_EN; // re-enable Rx interrupt

    return;
}


/**
 * \fn uint8_t GetArg(uint8_t *argIndex, uint8_t *indexRxRead)
 * \brief process receive buffer to extract arguments
 *
 * \param argIndex index of the current arg in the arg table, will be updated by the function
 * \param indexRxRead index of the current position in the RX buffer, will be updated by the function
 * \return LAST_ARG if the final character is CR, NEXT_ARG if there is still an arg to read, ERROR_ARG if conversion failed
 */
uint8_t GetArg(uint8_t *argIndex, uint8_t *indexRxRead)
{
    int8_t str[8];
    int8_t c; 
    uint8_t index = 0;

    // Extract digit str
    while((c = bufRx[*indexRxRead]))
    {
        if(c == '\r')
        {
            str[index++] = 0;
            args[*argIndex] = atol((char*)str);
            (*argIndex)++;
            return LAST_ARG;
        }
        else if(c == ' ')
        {
            str[index++] = 0;
            args[*argIndex] = atol((char*)str);
            (*argIndex)++;
            return NEXT_ARG;
        }
        else if(isdigit(c))
        {
            str[index++] = c;
        }
        else
        {
            return ERROR_ARG;
        }
        (*indexRxRead)++;
    }
    return ERROR_ARG;
}

/****
*
* CommandDispatch
*
* Parameters:
*       none
*
* Description:
*		This function implements answer to remote commands
*
* Returns:
*		OK if no error, else error
*
*/
uint8_t CommandDispatch(void)
{
    uint8_t result = OK;

    /* Process the command type */
    switch(command.cmdType)
    {
        case Cmd_Reset:
            // Hara Kiri !!
            // TODO : implement a clean reset not interfering with bootloader
            break;

        case Cmd_Version:
            args[0] = VERSION_SOFT_MAJOR;
            args[1] = VERSION_SOFT_MINOR;
            args[2] = VERSION_HARD;
            args[3] = VERSION_FOSSIL;
            command.nbArgSet = 4;
            SendToUsart();
            return result;
            break;

        case Cmd_Set:
        case Cmd_Get:
        case Cmd_Debug:
            // Check that command exists and the number of args 
            switch(args[0])
            {
                case Cmd_SetLed:
                    eData.timerLed = args[1];
                    command.nbArgSet = 1;
                    command.result = OK;
                    SendToUsart();
                    break;
            }
            break;

        default:
            args[0] = command.cmdType;
            command.nbArgSet = 1;
            command.cmdType = Cmd_Error;
            SendToUsart();
            result = BAD_COMMAND;
            return result;
    }

    switch(command.cmdId)
    {
        case 0:
        break;

        default:
            result = BAD_COMMAND;
    }

    return result;
}

/**
 * \fn void SendToUsart(void)
 * \brief send buffer to usart
 *
 * \param none
 * \return none
 */
void SendToUsart(void)
{
    uint8_t i;

    // write command type
    bufTx[indexTxWrite++] = command.cmdType;     
    bufTx[indexTxWrite++] = ' ';     
    // write args
    for(i=0; i<command.nbArgSet; i++)
    {
        SetArg(i);
        bufTx[indexTxWrite++] = ' ';     
    }
    // result if requested
    if(command.result)
    {
        bufTx[indexTxWrite++] = command.result;     
    }
    bufTx[indexTxWrite++] = '\r';     
    bufTx[indexTxWrite++] = '\n';     
    USART_TX_EN;
    return;
}

/**
 * \fn uint8_t SetArg(uint8_t argIndex)
 * \brief write args to output buffer 
 *
 * \param argIndex index of the current arg in the arg table
 * \return OK in case of succes
 */
uint8_t SetArg(uint8_t argIndex)
{
    uint8_t str[16];
    uint8_t *pstr = &str[0];

    ultoa(args[argIndex], str, 10);

    while(*pstr)
    {
        bufTx[indexTxWrite++] = *pstr++;
    }
    return OK;
}
