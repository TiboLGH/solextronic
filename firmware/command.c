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

const char revNum[20] =  {"Solextronic v0.2   "}; // TODO : Print HW/SW version from define 
const char signature[32] = {"** Solextronic v0.2 beta **    "};

/*** Commands ***/
extern eeprom_data_t    eData;
extern Current_Data_t   gState;


volatile u8 bufTx[BUFSIZE];
volatile u8 bufRx[BUFSIZE];
volatile u8 indexRx;
volatile u8 indexTxRead;
volatile u8 indexTxWrite;

static int32_t 	args[MAXARGS];
static commandType_t command;

u8 dbg[16];


static u8 GetArg(u8 *argIndex, u8 *indexRxRead);
static u8 SetArg(u8 argIndex);
static void SendToUsart(u8 *buffer, u8 len);

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
static u8 CommandDispatch(void)
{
    u16 curTime;

    /* Process the command type */
    switch(command.cmdType)
    {
        case 'a': // send back the internal state stored in gState
            SendToUsart((u8 *)&gState, sizeof(gState));
            return OK;

        case 'S': // send back the signature string
            SendToUsart((u8 *)signature, 32);
            return OK;

        case 'Q': // send back the revision string
            SendToUsart((u8 *)revNum, 20);
            return OK;

        case 'b': // burn parameters from RAM to EEPROM : faked, done in background
            return OK;

        case 'c': // send back the second counter 
            GetTime(&curTime);
            SendToUsart((u8*) &curTime, 2);
            return OK;

        case 't': // update a table
            break;

        case 'r': // read a particular parameter
            break;

        case 'w': // new parameter to write
            break;

        default:
            ASSERT(0);
    }

    return OK;
}

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
    u8 indexRxRead = 0;
    u8 argIndex = 0;
    u8 c;

    /* read reception buffer until last received char */
    while(indexRxRead != indexRx)
    {
        /* get character from reception buffer */
        c = bufRx[indexRxRead++];

        switch(state)
        {
            /* wait first valid character : a,S,Q,b,c,w,r,t characters */
            case IDLE:
                if(c == 'a' || c == 'S' || c == 'Q' || c == 'b' || c == 'c')
                { 
                    state = END; // no args
                }
                else if(c == 't' || c == 'r' || c == 'w')
                {
                    state = WAIT_ARG; // wait for args
                }
                command.cmdType = c;
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
            default:
                ASSERT(0); // should never reach this point
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

    return;
}

/**
 * \fn static void SendToUsart(u8 *buffer, u8 len)
 * \brief send buffer to usart
 *
 * \param u8 *buffer : pointer on the buffer to send 
 * \param u8 len : number of bytes to transmit starting from *buffer 
 * \return none
 */
static void SendToUsart(u8 *buffer, u8 len)
{
    u8 i, start = 0, bufEmpty = 0;

    if(indexTxWrite == indexTxRead) // nothing in buffer
    {
        start = indexTxWrite;
        indexTxRead = start + 1;
        bufEmpty = 1;
    }
    // copy buffer to send in circular buffer
    for(i=0; i < len; i++)
    {
        bufTx[indexTxWrite++] = *(buffer+i);     
    }
    if(bufEmpty) putchr(bufTx[start]); 
    USART_TX_EN;
    return;
}

/**
 * \fn u8 GetArg(u8 *argIndex, u8 *indexRxRead)
 * \brief process receive buffer to extract arguments
 *
 * \param argIndex index of the current arg in the arg table, will be updated by the function
 * \param indexRxRead index of the current position in the RX buffer, will be updated by the function
 * \return LAST_ARG if the final character is CR, NEXT_ARG if there is still an arg to read, ERROR_ARG if conversion failed
 */
static u8 GetArg(u8 *argIndex, u8 *indexRxRead)
{
    int8_t str[8];
    int8_t c; 
    u8 index = 0;

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

/**
 * \fn u8 SetArg(u8 argIndex)
 * \brief write args to output buffer 
 *
 * \param argIndex index of the current arg in the arg table
 * \return OK in case of succes
 */
static u8 SetArg(u8 argIndex)
{
    u8 str[16];
    u8 *pstr = &str[0];

    ultoa(args[argIndex], (char *)str, 10);

    while(*pstr)
    {
        bufTx[indexTxWrite++] = *pstr++;
    }
    return OK;
}
