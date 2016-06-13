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
 * \file trace_writer.c
 * \brief Trace file writer for EEPROM and current data structures. Highly inspired by sim_vcd_file.h/c from simavr. It also create a csv file for Tunerstudio compatible tools.
 * \author Thibault Bouttevin
 * \date May 2016
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "trace_writer.h"
#include "sim_avr.h"
#include "sim_time.h"
#include "varDescription.h"

//#define V(msg, ...) do{fprintf(stdout, "Trace_writer: "); fprintf(stdout, msg, ##__VA_ARGS__ );}while(0)
#define V(msg, ...) do{}while(0)

enum 
{
    DISABLE = 0,
    ENABLE
};

int trace_writer_init(struct avr_t * avr, const char * filename, trace_writer_t * vcd, uint32_t period, desc_t *test_members, int test_members_qty)
{
	memset(vcd, 0, sizeof(trace_writer_t));
	vcd->avr = avr;
	strncpy(vcd->filename, filename, sizeof(vcd->filename));
	vcd->period = avr_usec_to_cycles(vcd->avr, period);
    vcd->eeprom_desc_p = eeprom_desc;
    vcd->curData_desc_p = curData_desc;
    vcd->test_desc_p = test_members;
    vcd->test_qty = test_members_qty;
    vcd->fast_mode = DISABLE;

	return 0;
}

int trace_writer_fast_mode(trace_writer_t * vcd, int enable)
{
    vcd->fast_mode = enable?ENABLE:DISABLE;
    return 0;
}

void trace_writer_close(trace_writer_t * vcd)
{
	trace_writer_stop(vcd);
}

static uint32_t trace_writer_get_value(uint8_t *s, int offset, int size)
{
    uint32_t value = 0;
    switch(size)
    {
        case 8:
            value = s[offset];
        break;
        case 16:
            value = 256*s[offset+1] + s[offset];
        break;
        case 24:
            value = 256*256*s[offset+2] + 256*s[offset+1] + s[offset];
        break;
        case 32:
            value = 256*256*256*s[offset+3] + 256*256*s[offset+2] + 256*s[offset+1] + s[offset];
        break;
    }

    return value;
}

void trace_writer_signal_change(trace_writer_t *vcd, desc_t *signal, uint32_t value, uint64_t when)
{
	if (!vcd->output_vcd)
		return;
	/*
	 * buffer starts empty, the first trace will resize it to VCD_WRITER_LOG_CHUNK_SIZE,
	 * further growth will resize it accordingly. There's a bit of
	 */
	if (vcd->logindex >= vcd->logsize) {
		vcd->logsize += VCD_WRITER_LOG_CHUNK_SIZE;
		vcd->log = (trace_writer_log_p)realloc(vcd->log, vcd->logsize * sizeof(vcd->log[0]));
		V("%s trace buffer resized to %d\n",
				__func__, (int)vcd->logsize);
		if ((vcd->logsize / VCD_WRITER_LOG_CHUNK_SIZE) == 5) {
			V("%s log size runnaway (%d) flush problem?\n",
					__func__, (int)vcd->logsize);
		}
		if (!vcd->log) {
			V("%s log resizing, out of memory (%d)!\n",
					__func__, (int)vcd->logsize);
			vcd->logsize = 0;
			return;
		}
	}
	trace_writer_log_t *l = &vcd->log[vcd->logindex++];
	l->signal = signal;
	l->when = when;
	l->value = value;
}

int trace_writer_update_eeprom(trace_writer_t * vcd, eeprom_data_t *eData)
{
    int already_seen = -1; // to avoid raising several log for multibytes members
    /* compare current structure to cached one */
    uint8_t *cache = (uint8_t*)&(vcd->cached_eeprom);
    uint8_t *new = (uint8_t*)(eData);
    uint64_t when = vcd->avr->cycle;
    for(int i = 0; i < sizeof(eeprom_data_t); i++)
    {
        if(cache[i] != new[i])
        {
            //V("EEPROM : byte #%d changed !\n", i);
            // retrieve corresponding member
            int j;
            for(j = 0; j < EEPROM_QTY; j++)
            {
                if(i < vcd->eeprom_desc_p[j].offset) break;
            }
            if(already_seen == (j-1)) continue;
            already_seen = j-1;
            desc_t *signal = &(vcd->eeprom_desc_p[j-1]);
            // raise signal change
            uint32_t value = trace_writer_get_value(new, signal->offset, signal->size);
            signal->value = value;
            V("EEPROM : raise log on member %d : %s = %d\n", j-1, signal->name, value);
            trace_writer_signal_change(vcd, signal, value, when);
        }
    }
    memcpy(&(vcd->cached_eeprom), eData, sizeof(eeprom_data_t));

    return 0;
}

int trace_writer_update_curData(trace_writer_t * vcd, current_data_t *curData)
{
    int already_seen = -1; // to avoid raising several log for multibytes members
    /* compare current structure to cached one */
    uint8_t *cache = (uint8_t*)&(vcd->cached_curData);
    uint8_t *new = (uint8_t*)(curData);
    uint64_t when = vcd->avr->cycle;
    for(int i = 0; i < sizeof(current_data_t); i++)
    {
        if(cache[i] != new[i])
        {
            V("gState : byte #%d changed !\n", i);
            // retrieve corresponding member
            int j;
            for(j = 0; j < CURDATA_QTY; j++)
            {
                if(i < vcd->curData_desc_p[j].offset) break;
            }
            if(already_seen == (j-1)) continue;
            already_seen = j-1;
            desc_t *signal = &(vcd->curData_desc_p[j-1]);
            // raise signal change
            uint32_t value = trace_writer_get_value(new, signal->offset, signal->size);
            signal->value = value;
            V("gState : raise log on member %d : %s = %d\n", j-1, signal->name, value);
            trace_writer_signal_change(vcd, signal, value, when);
        }
    }
    memcpy(&(vcd->cached_curData), curData, sizeof(current_data_t));

    return 0;
}

static char * _trace_writer_get_signal_text(desc_t * s, char * out, uint32_t value)
{
	char * dst = out;

	if (s->size > 1)
		*dst++ = 'b';

	for (int i = s->size; i > 0; i--)
		*dst++ = value & (1 << (i-1)) ? '1' : '0';
	if (s->size > 1)
		*dst++ = ' ';
	strcpy(dst, s->alias);
    //*dst++ = s->alias;
	//*dst = 0;
	return out;
}

static void trace_writer_flush_log(trace_writer_t * vcd)
{
	uint64_t oldbase = 0;	// make sure it's different
	char out[48];

	if (!vcd->logindex || !vcd->output_vcd)
		return;


	for (uint32_t li = 0; li < vcd->logindex; li++) {
		trace_writer_log_t *l = &vcd->log[li];
		uint64_t base = avr_cycles_to_nsec(vcd->avr, l->when - vcd->start);	// 1ns base

		if (base > oldbase || li == 0) {
			fprintf(vcd->output_vcd, "#%" PRIu64  "\n", base);
            // dump all in s32. TODO : follow format qualifier to write the right format
            char buf[8192];
            sprintf(buf, "%u", (uint32_t)(base /1000)); //us
            for (int i = 0; i < EEPROM_QTY; i++) {
                if(vcd->eeprom_desc_p[i].size == -1) continue; // skip array for now
                sprintf(buf, "%s;%d", buf, vcd->eeprom_desc_p[i].value);
            }
            for (int i = 0; i < CURDATA_QTY; i++) {
                if(vcd->curData_desc_p[i].size == -1) continue; // skip array for now
                sprintf(buf, "%s;%d", buf, vcd->curData_desc_p[i].value);
            }
            for (int i = 0; i < vcd->test_qty; i++) {
                sprintf(buf, "%s;%d", buf, vcd->test_desc_p[i].value);
            }
            fprintf(vcd->output_csv, "%s;\n", buf);
            
			oldbase = base;
		}
		if(l->signal->size != -1) fprintf(vcd->output_vcd, "%s\n", _trace_writer_get_signal_text(l->signal, out, l->value));
	}
	vcd->logindex = 0;
}

static avr_cycle_count_t _trace_writer_timer(struct avr_t * avr, avr_cycle_count_t when, void * param)
{
	trace_writer_t * vcd = param;
	trace_writer_flush_log(vcd);
	return when + vcd->period;
}

int trace_writer_start(trace_writer_t * vcd, eeprom_data_t *initEe, current_data_t *initCur)
{
	if (vcd->output_vcd)
		trace_writer_stop(vcd);
	//CSV file
    vcd->output_vcd = fopen(vcd->filename, "w");
	if (vcd->output_vcd == NULL) {
		perror(vcd->filename);
		return -1;
	}

	vcd->output_vcd = fopen(vcd->filename, "w");
	if (vcd->output_vcd == NULL) {
		perror(vcd->filename);
		return -1;
	}
	//CSV file
    char csv_filename[80];
    snprintf(csv_filename, 80, "%s.csv", vcd->filename);
    vcd->output_csv = fopen(csv_filename, "w");
	if (vcd->output_csv == NULL) {
		perror(csv_filename);
		return -1;
	}

    // header for vcd
	fprintf(vcd->output_vcd, "$timescale 1ns $end\n");	// 1ns base
	fprintf(vcd->output_vcd, "$scope module eData $end\n");
	for (int i = 0; i < EEPROM_QTY; i++) {
        if(vcd->eeprom_desc_p[i].size == -1) continue; // skip array for now
		fprintf(vcd->output_vcd, "$var wire %d %s %s $end\n",
			vcd->eeprom_desc_p[i].size, vcd->eeprom_desc_p[i].alias, vcd->eeprom_desc_p[i].name);
	}
	fprintf(vcd->output_vcd, "$upscope $end\n");
	fprintf(vcd->output_vcd, "$scope module gState $end\n");
	for (int i = 0; i < CURDATA_QTY; i++) {
        if(vcd->curData_desc_p[i].size == -1) continue; // skip array for now
		fprintf(vcd->output_vcd, "$var wire %d %s %s $end\n",
			vcd->curData_desc_p[i].size, vcd->curData_desc_p[i].alias, vcd->curData_desc_p[i].name);
	}
	fprintf(vcd->output_vcd, "$upscope $end\n");
	fprintf(vcd->output_vcd, "$scope module test $end\n");
	for (int i = 0; i < vcd->test_qty; i++) {
		fprintf(vcd->output_vcd, "$var wire %d %s %s $end\n",
			vcd->test_desc_p[i].size, vcd->test_desc_p[i].alias, vcd->test_desc_p[i].name);
	}
	fprintf(vcd->output_vcd, "$upscope $end\n");
    fprintf(vcd->output_vcd, "$enddefinitions $end\n");

    // init cached structures
    memcpy(&(vcd->cached_eeprom), initEe, sizeof(eeprom_data_t));
    memcpy(&(vcd->cached_curData), initCur, sizeof(current_data_t));
	fprintf(vcd->output_vcd, "$dumpvars\n");
    char out[48];
	for (int i = 0; i < EEPROM_QTY; i++) {
		desc_t * s = &vcd->eeprom_desc_p[i];
        if(s->size == -1) continue;
        uint32_t value = trace_writer_get_value((uint8_t*)initEe, s->offset, s->size);
		fprintf(vcd->output_vcd, "%s\n", _trace_writer_get_signal_text(s, out, value));
	}
	for (int i = 0; i < CURDATA_QTY; i++) {
		desc_t * s = &vcd->curData_desc_p[i];
        if(s->size == -1) continue;
        uint32_t value = trace_writer_get_value((uint8_t*)initCur, s->offset, s->size);
		fprintf(vcd->output_vcd, "%s\n", _trace_writer_get_signal_text(s, out, value));
	}
	for (int i = 0; i < vcd->test_qty; i++) {
		desc_t * s = &vcd->test_desc_p[i];
        if(s->size == -1) continue;
        uint32_t value = trace_writer_get_value((uint8_t*)initEe, s->offset, s->size);
		fprintf(vcd->output_vcd, "%s\n", _trace_writer_get_signal_text(s, out, value));
	}
	fprintf(vcd->output_vcd, "$end\n");

    //header for csv
    char buf[8192];
	for (int i = 0; i < EEPROM_QTY; i++) {
        if(vcd->eeprom_desc_p[i].size == -1) continue; // skip array for now
		sprintf(buf, "%s;%s", buf, vcd->eeprom_desc_p[i].name);
	}
	for (int i = 0; i < CURDATA_QTY; i++) {
        if(vcd->curData_desc_p[i].size == -1) continue; // skip array for now
		sprintf(buf, "%s;%s", buf, vcd->curData_desc_p[i].name);
	}
	for (int i = 0; i < vcd->test_qty; i++) {
		sprintf(buf, "%s;%s", buf, vcd->test_desc_p[i].name);
	}
    fprintf(vcd->output_csv, "%s;\n", buf);


    vcd->logindex = 0;
	vcd->start = vcd->avr->cycle;
	avr_cycle_timer_register(vcd->avr, vcd->period, _trace_writer_timer, vcd);
	return 0;
}

int trace_writer_stop(trace_writer_t * vcd)
{
	avr_cycle_timer_cancel(vcd->avr, _trace_writer_timer, vcd);

	trace_writer_flush_log(vcd);

	if (vcd->output_vcd)
		fclose(vcd->output_vcd);
	vcd->output_vcd = NULL;
	if (vcd->output_csv)
		fclose(vcd->output_csv);
	vcd->output_csv = NULL;
	return 0;
}


