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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "logger.h" 

typedef struct{
	int handle;
	char name[32];
	e_level level;
	int color;
} loghandler_t;

static int colorCode[COLOR_QTY] = {
0, //COLOR_AUTO = 0,
97, //COLOR_WHITE,
31, //COLOR_RED,
32, //COLOR_GREEN,
33, //COLOR_YELLOW,
34, //COLOR_BLUE,
35, //COLOR_MAGENTA,
36, //COLOR_CYAN,
37, //COLOR_LIGHT_GREY,
90, //COLOR_DARK_GREY,
91, //COLOR_LIGHT_RED,
92, //COLOR_LIGHT_GREEN,
93, //COLOR_LIGHT_YELLOW,
94, //COLOR_LIGHT_BLUE,
95, //COLOR_LIGHT_MAGENTA,
96, //COLOR_LIGHT_CYAN,
};

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
		if(filename)
		{
			if((pf = fopen(filename, "wb")))
			{
				toFile = true;
			}else{
				printf("File creation failed !\n");
				return -1;
			}
			printf("No filename ! \n");
			return -1;
		}
    }
	return 0;
}

/* Client registration : */
int logger_register(char *name, e_color color, e_level level)
{
	if(logQty > (MAX_LOG-1))
	{
		printf("Max log qty !");
		return -1;
	}

	logHandlers[logQty].handle 	= logQty;
	logHandlers[logQty].level 	= level;
	logHandlers[logQty].color 	= colorCode[(color == COLOR_AUTO) ? (logQty+1) % COLOR_QTY : color];
	strncpy(logHandlers[logQty].name, name, 32);
	if(strlen(name) > longuestName) longuestName = strlen(name);
	logQty++;
	return (logQty-1);
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
		logHandlers[index].level = level;
		return 0;
	}
    return -1;
}	

/* Log : */
int logger_log(e_level level, int handle, char *msg, ... )
{
    if(handle > logQty) return -1;
    if(logHandlers[handle].level < level) return 0;

    char buf[256];
    va_list ap;
    va_start(ap, msg);
    vsnprintf(buf, 256, msg, ap);
    va_end(ap);

    if(toFile)
    {
        fprintf(pf, "%s > %s", logHandlers[handle].name, buf);
    }
    if(toStdout)
    {
        fprintf(stdout, "\033[%dm%s \033[0m> %s", 
                logHandlers[handle].color,
                logHandlers[handle].name, 
                buf);
    }
    return 0;
}

/*Explicit flush : */
int logger_flush(void)
{
    if(toFile) fflush(pf);
    if(toStdout) fflush(stdout);
    return 0;
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

