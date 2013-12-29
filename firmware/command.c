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

//const char revNum[20] =  {"Solextronic v0.2   "}; // TODO : Print HW/SW version from define 
const char revNum[20] =  {"MSII Rev 2.90500    "};   
const char signature[32] = {"** Solextronic v0.2 beta **     "};
const char protocol[3] = {"001"};

/*** Commands ***/
extern eeprom_data_t    eData;
extern Current_Data_t   gState;
static Current_Data_t   outputChannel;


volatile u8 *bufTx;
volatile u8 bufRx[BUFSIZE];
volatile u8 indexRx;
volatile u16 txCur = 0;
volatile u16 txCount = 0;
volatile u8 isTx = False;

u8 dbg[16];


static void SendToUsart(const u8 *buffer, const u16 len);

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
    static state_e state = IDLE;
    static u8  indexRxRead = 0;
    static u8  command   = 0;
    static u16 offset    = 0;
    static u16 nbData    = 0;
    static u16 dataCount = 0;
    u8 c;
    u16 curTime; 

    /* read reception buffer until last received char */
    while(indexRxRead != indexRx)
    {
        /* get character from reception buffer */
        c = bufRx[indexRxRead++];

        switch(state)
        {
            /* wait first valid character : A,S,Q,b,c,w,r,t characters */
            case IDLE:
                switch(c)
                {
                    case 'a':
                    case 'A': // send back the internal state stored in gState
                        outputChannel = gState; // copy in outputChannel to avoid change during transfert
                        SendToUsart((u8 *)&outputChannel, sizeof(gState));
                        break;

                    case 'S': // send back the signature string
                        SendToUsart((u8 *)signature, 32);
                        break;

                    case 'Q': // send back the revision string
                        SendToUsart((u8 *)revNum, 20);
                        break;

                    case 'F': // send back the protocol
                        SendToUsart((u8 *)protocol, 3);
                        break;

                    case 'c': // send back the second counter
                        GetTime(&curTime);
                        SendToUsart((u8*) &curTime, 2);
                        break;
                    
                    case 'b': // burn parameters from RAM to EEPROM : faked, done in background
                        break;

                    case 't': // TODO update a table
                        offset = 0;
                        nbData = 0;
                        state = DATA;
                        break;

                    case 'y': // check writing in flash => always successful
                        SendToUsart((u8 *)"0", 1);
                        break;

                    case 'r': // read a particular parameter 
                    case 'e': // update a table
                    case 'w': // new parameter to write
                        state = OFFSET_L; // wait for args
                        break;
                }
                command = c;
                break;

            case OFFSET_L:
                offset = c;
                state = OFFSET_H;
            break;

            case OFFSET_H:
                offset += ((u16)c) << 8;
                state = NUM_L;
            break;

            case NUM_L:
                nbData = c;
                state = NUM_H;
            break;

            case NUM_H:
                nbData += ((u16)c) << 8;
                if(command == 'r')
                {
                    ASSERT(nbData + offset <= sizeof(eData)); 
                    SendToUsart((u8 *)&eData + offset, nbData);
                    state = IDLE;
                }else{
                    state = DATA;
                    dataCount = 0;
                }
            break;

            case DATA:
                if(command == 'e' || command == 'w')
                {
                    //write data
                    *((u8*)&eData + offset + dataCount) = c;
                    dataCount++;
                    if(dataCount == nbData)
                    {
                        state = IDLE;
                        if(command == 'e') SendToUsart((u8 *)&eData + offset, nbData);
                    } 
                }else{
                    state = IDLE;
                }
                break;
            default:
                ASSERT(0); // should never reach this point
        }// switch state
    }// while

    return;
}

/**
 * \fn static void SendToUsart(u8 *buffer, u16 len)
 * \brief send buffer to usart
 *
 * \param u8 *buffer : pointer on the buffer to send 
 * \param u16 len : number of bytes to transmit starting from *buffer 
 * \return none
 */
static void SendToUsart(const u8 *buffer, const u16 len)
{
    ASSERT(len);
    ASSERT(!isTx); // Collision !
   
    isTx = True; 
    txCount = len;
    txCur = 1;
    bufTx = buffer;
    putchr(*bufTx); // manually put 1st character 
    USART_TX_EN;
    return;
}
