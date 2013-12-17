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

#define Cmd_SetGeneral      1
#define Cmd_SetIgnition     2
#define Cmd_SetInjection    3
#define Cmd_SetInjectionTable    4
#define Cmd_SetIgnitionTable     5
#define Cmd_SetRatio        6
#define Cmd_SetPolarity     7
#define Cmd_SetHT           8
#define Cmd_SetLed          11
#define Cmd_GetStatus       11
#define Cmd_GetGeneral      12 
#define Cmd_GetADC          13 
#define Cmd_GetIgnition     14 
#define Cmd_GetInjection    15 

extern Flags_t          Flags;
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
static void SendToUsart(u8 result, u8 nbArg, u8 type);

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
    u8 result = OK;

    /* Process the command type */
    switch(command.cmdType)
    {
        case Cmd_Reset:
            // Hara Kiri !!
            // TODO : implement a clean reset not interfering with bootloader
			ASSERT(NULL);
            break;

        case Cmd_Version:
            args[0] = VERSION_SOFT_MAJOR;
            args[1] = VERSION_SOFT_MINOR;
            args[2] = VERSION_HARD;
            args[3] = VERSION_GIT;
            SendToUsart(OK, 4, command.cmdType);
            return result;
            break;

        case Cmd_Set:
        case Cmd_Debug:
            // Check that command exists and the number of args 
            switch(args[0])
            {
                case Cmd_SetGeneral:
                    eData.wheelSize = args[1];
                    eData.PMHOffset = args[2];
                    eData.maxRPM    = args[3];
                    eData.maxTemp   = args[4];
                    eData.minBat    = args[5];
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                case Cmd_SetIgnition:
                    eData.igniDuration = args[1];
                    eData.starterAdv   = args[2];
                    eData.igniOverheat = args[3];
                    eData.noSparkAtDec = args[4];
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                case Cmd_SetInjection:
                    eData.injOpen      = args[1];
                    eData.injRate      = args[2];
                    eData.injAdv       = args[3];
                    eData.starterInj   = args[4];
                    eData.injOverheat  = args[5];
                    eData.injFullOpen  = args[6];
                    eData.noInjAtDec   = args[7];
                    eData.injStart     = args[8];
                    eData.holdPWM      = args[9];
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                case Cmd_SetPolarity:
                    eData.igniPolarity = args[1];
                    eData.injPolarity  = args[2];
                    eData.pmhPolarity  = args[3];
                    eData.pumpPolarity = args[4];
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                case Cmd_SetInjectionTable:
                    if((args[1] > 12) || (args[2] > 12))
                    {
                        SendToUsart(BAD_PARAMETER, 1, command.cmdType);
                    }else{
                        eData.injTable[args[1]][args[2]]  = args[3];
                        SendToUsart(OK, 1, command.cmdType);
                    }
                    break;
                case Cmd_SetIgnitionTable:
                    if((args[1] > 12) || (args[2] > 12))
                    {
                        SendToUsart(BAD_PARAMETER, 1, command.cmdType);
                    }else{
                        eData.igniTable[args[1]][args[2]]  = args[3];
                        SendToUsart(OK, 1, command.cmdType);
                    }
                    break;
                case Cmd_SetHT:
                    eData.HVstep = args[1];
                    eData.HVmanual = args[2];
                    if(eData.HVstep == 0)   WritePWMValue(eData.HVmanual);
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                case Cmd_SetLed:
                    eData.timerLed = args[1];
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                case Cmd_SetRatio:
                    eData.ratio[0] = args[1];
                    eData.ratio[1] = eData.ratio[2] = args[2];
                    eData.ratio[3] = args[3];
                    eData.ratio[4] = args[4];
                    SendToUsart(OK, 1, command.cmdType);
                    break;
                default:
                    SendToUsart(BAD_COMMAND, 1, Cmd_Error);
            }
            break;
        
        case Cmd_Get:
            // Check that command exists and the number of args 
            switch(args[0])
            {
                case Cmd_SetGeneral:
                    args[1] = eData.wheelSize;
                    args[2] = eData.PMHOffset;
                    args[3] = eData.maxRPM;
                    args[4] = eData.maxTemp;
                    args[5] = eData.minBat;
                    SendToUsart(OK, 6, command.cmdType);
                    break;
                case Cmd_SetIgnition:
                    args[1] = eData.igniDuration;
                    args[2] = eData.starterAdv;
                    args[3] = eData.igniOverheat;
                    args[4] = eData.noSparkAtDec;
                    SendToUsart(OK, 5, command.cmdType);
                    break;
                case Cmd_SetInjection:
                    args[1] = eData.injOpen;
                    args[2] = eData.injRate;
                    args[3] = eData.injAdv;
                    args[4] = eData.starterInj;
                    args[5] = eData.injOverheat;
                    args[6] = eData.injFullOpen;
                    args[7] = eData.noInjAtDec;
                    args[8] = eData.injStart;
                    args[9] = eData.holdPWM;
                    SendToUsart(OK, 10, command.cmdType);
                    break;
                case Cmd_SetPolarity:
                    args[1] = eData.igniPolarity;
                    args[2] = eData.injPolarity;
                    args[3] = eData.pmhPolarity;
                    args[4] = eData.pumpPolarity;
                    SendToUsart(OK, 5, command.cmdType);
                    break;
                case Cmd_SetInjectionTable:
                    if((args[1] > 12) || (args[2] > 12))
                    {
                        SendToUsart(BAD_PARAMETER, 1, command.cmdType);
                    }else{
                        args[3] = eData.injTable[args[1]][args[2]];
                        SendToUsart(OK, 2, command.cmdType);
                    }
                    break;
                case Cmd_SetIgnitionTable:
                    if((args[1] > 12) || (args[2] > 12))
                    {
                        SendToUsart(BAD_PARAMETER, 1, command.cmdType);
                    }else{
                        args[3] = eData.igniTable[args[1]][args[2]];
                        SendToUsart(OK, 2, command.cmdType);
                    }
                    break;
                case Cmd_SetHT:
                    args[1] = eData.HVstep;
                    args[2] = eData.HVmanual;
                    args[3] = gState.HVvalue;
                    SendToUsart(OK, 4, command.cmdType);
                    break;
                case Cmd_SetRatio:
                    args[1]= eData.ratio[0];
                    args[2]= eData.ratio[1];
                    args[3]= eData.ratio[3];
                    args[4]= eData.ratio[4];
                    SendToUsart(OK, 5, command.cmdType);
                    break;
                case Cmd_GetGeneral:
                    args[1] = gState.engine;
                    args[2] = gState.RPM;
                    args[3] = gState.speed;
                    SendToUsart(OK, 4, command.cmdType);
                    break;
                case Cmd_GetADC:
                    args[1] = gState.battery;
                    args[2] = gState.temp1;
                    args[3] = gState.temp2;
                    args[4] = gState.throttle;
                    args[5] = gState.pressure;
                    SendToUsart(OK, 6, command.cmdType);
                    break;
                case Cmd_SetLed:
                    args[1] = eData.timerLed;
                    SendToUsart(OK, 2, command.cmdType);
                    break;
                default:
                    SendToUsart(BAD_COMMAND, 1, Cmd_Error);
            }
            break;

        default:
            args[0] = command.cmdType;
            SendToUsart(BAD_COMMAND, 1, Cmd_Error);
            result = BAD_COMMAND;
            return result;
    }

    return result;
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
    USART_RX_EN; // re-enable Rx interrupt

    return;
}

/**
 * \fn void SendToUsart(u8 result, u8 nbArg, u8 type)
 * \brief send buffer to usart
 *
 * \param none
 * \return none
 */
static void SendToUsart(u8 result, u8 nbArg, u8 type)
{
    u8 i, start = 0, bufEmpty = 0;

    if(indexTxWrite == indexTxRead) // nothing in buffer
    {
        start = indexTxWrite;
        indexTxRead = start + 1;
        bufEmpty = 1;
    }
    // write command type
    bufTx[indexTxWrite++] = type;     
    bufTx[indexTxWrite++] = ' ';     
    // write args
    for(i=0; i<nbArg; i++)
    {
        SetArg(i);
        bufTx[indexTxWrite++] = ' ';     
    }
    // result if requested
    if(result)
    {
        bufTx[indexTxWrite++] = command.result;     
    }
    bufTx[indexTxWrite++] = '\r';     
    bufTx[indexTxWrite++] = '\n';    
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
