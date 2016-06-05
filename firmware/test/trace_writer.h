/***************************************************************************
 *   Copyright (C) 2013 by Thibault Bouttevin                              *
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
 * \file trace_writer.h
 * \brief Trace file writer for EEPROM and current data structures. Highly inspired by sim_vcd_file.h/c from simavr. It also create a csv file for Tunerstudio compatible tools.
 * \author Thibault Bouttevin
 * \date May 2016
 *
 */
#ifndef VCD_WRITER_H
#define VCD_WRITER_H

#include <stdio.h>
#include "sim_irq.h"
#include "../varDef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[32];
    int size;
    char alias[3]; // to support > 90 vars
    int offset;
    char type[8];
    uint32_t value;
    int seen;
}desc_t;

typedef struct trace_writer_log_t {
	uint64_t 	when;
	desc_t *signal;
	uint32_t value;
} trace_writer_log_t, *trace_writer_log_p;

#define VCD_WRITER_LOG_CHUNK_SIZE	(4096 / sizeof(desc_t))

typedef struct trace_writer_t {
	struct avr_t *	avr;	// AVR we are attaching timers to..
	
	char filename[74];		// output filename
	FILE * output_vcd, *output_csv;

	int signal_count;
	desc_t *eeprom_desc_p;
    eeprom_data_t cached_eeprom;
	desc_t *curData_desc_p;
    current_data_t cached_curData;
	desc_t *test_desc_p;
    int test_qty;


	uint64_t period;
	uint64_t start;
    int fast_mode;

	size_t			logsize;
	uint32_t		logindex;
	trace_writer_log_p	log;
} trace_writer_t;

// initializes a new VCD trace file, and returns zero if all is well
int trace_writer_init(struct avr_t * avr, 
	const char * filename, 	// filename to write
	trace_writer_t * vcd,		// vcd struct to initialize
	uint32_t	period, 	// file flushing period is in usec
    desc_t *test_members,   // test fields description table
    int test_members_qty);  // test fields quantity
void trace_writer_close(trace_writer_t * vcd);

int trace_writer_update_eeprom(trace_writer_t * vcd, eeprom_data_t *eData);
int trace_writer_update_curData(trace_writer_t * vcd, current_data_t *curData);
int trace_writer_update_test(trace_writer_t * vcd);

// Starts recording the signal value into the file
int trace_writer_start(trace_writer_t * vcd, eeprom_data_t *initEe, current_data_t *initCur);
// Start/stop fast mode : query cureent data as fast as possible
int trace_writer_fast_mode(trace_writer_t * vcd, int enable);
// stops recording signal values into the file
int trace_writer_stop(trace_writer_t * vcd);

#ifdef __cplusplus
};
#endif

#endif /* VCD_WRITER_H */
