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
 * \file command.h
 * \brief Manage USART command interpreter
 * \author Thibault Bouttevin
 * \date October 2012
 *
 * This file implements commands interpretation and serial port buffers management.
 *
 */


#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

#define BUFSIZE      256
#define MAXARGS    	 10

/* macro to manage buffer pointer */
#define INCPTR(X) ((X)==(BUFSIZE-1) ? ((X)=0) : (X)++)

enum
{
   VOID = 0,
   OK,
   BAD_PARAMETER,
   BAD_LENGTH, 
   BAD_COMMAND,
   LAST_ARG,
   NEXT_ARG,
   ERROR_ARG
};

typedef enum
{
    IDLE = 0,
    WAIT_CR,
    WAIT_ARG,
    END
} state_e;

/*** Command Types ***/
#define Cmd_Reset          'r'
#define Cmd_Version	       'v'
#define Cmd_Set            's'
#define Cmd_Get            'g'
#define Cmd_Debug          'd'
#define Cmd_Error          'e'

typedef struct
{
	uint8_t		cmdType;
	uint8_t		cmdId;
	int8_t		nbArgSet;
	int8_t		nbArgGet;
	int8_t		result;
} commandType_t;

void ProcessCommand(void);

#endif // COMMAND_H
