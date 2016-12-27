/***************************************************************************
 *   Copyright (C) 2016 by Thibault Bouttevin                              *
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
 * \file logger.c
 * \brief Centralized logging system 
 * \author Thibault Bouttevin
 * \date December 2016
 *
 */

#include "logger.h" 

typedef struct{
	int handle;
	char name[32];
	e_level level;
	e_color color;
} loghandler_t;

#define MAX_LOG (16)
static loghandler_t logHandlers[MAX_LOG];
static int logQty = 0;
static FILE *pf = NULL;
static bool toStdout = true;
static bool toFile = false;
static int longuestName = 0;

/* Initialization : */
int logger_init(int options, char *filename)
{
	if(options & TO_STDOUT)
	{
		toStdout = true;
	}
	
	if(options & TO_FILE)
	{
		if(fileName)
		{
			if(pf = fopen(filename, "wb"))
			{
				toFile = true;
			}else{
				printf("File creation failed !\n");
				return -1;
			}
			print("No filename ! \n")
			return -1;
		}
	return 0;
}

/* Client registration : */
int logger_register(char *name, e_color color, e_level level)
{
	if(logQty < (MAX_LOG-1))
	{
		printf("Max log qty !");
		return -1;
	}

	logQty++;
	logHandlers[logQty].handle 	= logQty + 1;
	logHandlers[logQty].level 	= level;
	logHandlers[logQty].color 	= (color == COLOR_AUTO) ? logQty : color;
	strncpy(logHandlers[logQty].name, name, 32);
	if(strlen(name) > longuestName) longuestName = strlen(name);
	return 0;
}

/* Verbosity control : */
int logger_level(char *name, e_level level)
{
	int index = -1;
	for(int i=0; i<logQty; i++)
	{
		if(strcmp(name, logHandlers[i].name) == 0) 
		{
			index = i;
			break;
		}
	}
	if(index != -1)
	{
		logHandlers[index].level 	= level;
		return 0;
	}else{
		return -1;
	}
}	
/* Log : */
int logger_error(int handle, char *msg, args);
int logger_warn(int handle, char *msg, args);
int logger_info(int handle, char *msg, args);
int logger_dbg(int handle, char *msg, args);
#define ERROR(logHandle, msg, ...) do{logger_error(logHandle, msg, ##__VA_ARGS__ )}while(0)
#define WARN(logHandle, msg, ...) do{logger_warn(logHandle, msg, ##__VA_ARGS__ )}while(0)
#define INFO(logHangle, msg, ...) do{logger_info(logHandle, msg, ##__VA_ARGS__ )}while(0)
#define DBG(logHandle, msg, ...) do{logger_dbg(logHandle, msg, ##__VA_ARGS__ )}while(0)

/*Explicit flush : */
int logger_flush(void)
{

}
int logger_close(void)
{
	if(pf)
	{
		fclose(pf);
		pf = NULL;
	}
	return 0;
}
