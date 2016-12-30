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
 * \file logger.h
 * \brief Centralized logging system 
 * \author Thibault Bouttevin
 * \date December 2016
 *
 */

/**
 * Interface
 *
 * Initialization : logger_init(option : FILE and/or terminal, filename);
 * Client registration : logHandle = logger_register(name, color);
 * Verbosity control : logger_level(name, level); // level = LOGGER_DBG, LOGGER_INFO, LOGGER_WARN, LOGGER_ERROR
 * Log : ERROR(logHandle, msg), WARN(logHandle, msg), INFO(logHangle, msg), DBG(logHandle, msg)
 * Explicit flush : logger_flush();
 */

typedef enum{
	COLOR_AUTO = 0,
	COLOR_WHITE,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_LIGHT_GREY,
	COLOR_DARK_GREY,
	COLOR_LIGHT_RED,
	COLOR_LIGHT_GREEN,
	COLOR_LIGHT_YELLOW,
	COLOR_LIGHT_BLUE,
	COLOR_LIGHT_MAGENTA,
	COLOR_LIGHT_CYAN,
	COLOR_QTY
} e_color;

typedef enum{
	LOGGER_ERROR = 0,
	LOGGER_WARN,
	LOGGER_INFO,
	LOGGER_DBG,
	LOGGER_QTY,
} e_level;

#define TO_FILE 1
#define TO_STDOUT 2

/* Initialization : */
int logger_init(int options, char *filename);
/* Client registration : */
int logger_register(char *name, e_color color, e_level level);
/* Verbosity control : */
int logger_level(char *name, e_level level); // level = LOGGER_DBG, LOGGER_INFO, LOGGER_WARN, LOGGER_ERROR
/* Log : */
int logger_log(e_level level, int handle, char *msg, ...);
#define ERROR(LOGhANDLE, mSg, ...) do{logger_log(LOGGER_ERROR, LOGhANDLE, mSg, ##__VA_ARGS__ );}while(0)
#define WARN(LOGhANDLE, mSg, ...) do{logger_log(LOGGER_WARN, LOGhANDLE, mSg, ##__VA_ARGS__ );}while(0)
#define INFO(LOGhANDLE, mSg, ...) do{logger_log(LOGGER_INFO, LOGhANDLE, mSg, ##__VA_ARGS__ );}while(0)
#define DBG(LOGhANDLE, mSg, ...) do{logger_log(LOGGER_DBG, LOGhANDLE, mSg, ##__VA_ARGS__ );}while(0)

/* Explicit flush : */
int logger_flush(void);
/* Close logger : */
int logger_close(void);

