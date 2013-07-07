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
 * \file simMain.c
 * \brief Main file of the simulation
 * \author Thibault Bouttevin
 * \date June 2013
 *
 * This file includes main function and main loop
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_timer.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_vcd_file.h"
#include "sim_irq.h"
#include "sim_time.h"
#include "uart_pty.h"
#include "pulse_input.h"

#include <pthread.h>

avr_t * avr = NULL;
avr_vcd_t vcd_file;
uart_pty_t uart_pty;
pulse_input_t pulse_input_engine;
pulse_input_t pulse_input_wheel;

volatile uint8_t	display_pwm = 0;

/********* Helpers ************/
static void RPMtoPeriod(const uint32_t rpm, uint32_t *tHigh, uint32_t *tLow)
{
    *tHigh = 100; /* set high duration to 100us */
    *tLow  = 1000000 / (rpm / 60) - *tHigh;
}

static void SpeedtoPeriod(const uint32_t speed, uint32_t *tHigh, uint32_t *tLow)
{
    *tHigh = 1000; /* set high duration to 1ms */
    *tLow  = 1000000 / (speed / 3.6 / 1.82) - *tHigh;
}

static void pwm_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	display_pwm = value;
}

/**************************************/
static void *avr_run_thread(void * oaram)
{
	while (1) {
		int state = avr_run(avr);
		if ( state == cpu_Done || state == cpu_Crashed)
        {
            printf("fin du thread state = %d\n", state);
			break;
        }
	}
	return NULL;
}

/* Print help an exit with exit code exit_msg */
static void printHelp(FILE *stream, int exitMsg, const char* progName)
{
	fprintf(stream,"usage : %s [options] elfFile\n", progName);
	fprintf(stream,"Les options valides sont :\n");
	fprintf(stream,
	"  -h\t\t affiche ce message\n"
	"  -m\t\t mode manuel\n"
	"  -l\t\t liste des tests disponibles\n"
    "  -a\t\t lancement de tous les tests\n"
    "  -t <test>\t\t lancement du test <test>\n"
    );
	exit(exitMsg);
}

/********* Main ************/
int main(int argc, char *argv[])
{
    char elfName[256], testName[256];
    /* read command-line arguments */
    int c;
    opterr = 0;
    if(argc == 1)
    {
        printHelp(stdout, EXIT_SUCCESS, argv[0]);
    }
    while ((c = getopt (argc, argv, "hlmat:")) != -1)
    {
        switch (c)
        {
            case 'h': // help
                printHelp(stdout, EXIT_SUCCESS, argv[0]);
                break;
            case 'l': // list of available tests
                break;
            case 'm': // manual mode
                break;
            case 'a': // run all tests
                break;
            case 't': // run test <test>
                strncpy(testName, optarg, 256);
                break;
            case '?':
                if (optopt == 't')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                printHelp(stderr, EXIT_FAILURE, argv[0]);
            default:
                abort();
        }
    }
    if((optind == argc) || (argc - optind > 1))
    {
        fprintf (stderr, "ELF file is mandatory !\n");
        printHelp(stderr, EXIT_FAILURE, argv[0]);
    }
    strncpy(elfName, argv[optind], 256);


	elf_firmware_t f;
	const char * fname = "../solextronic.elf";
	elf_read_firmware(fname, &f);
    
    strcpy(f.mmcu, "atmega328");
    f.frequency = 16000000;
	avr = avr_make_mcu_by_name(f.mmcu);
	if (!avr) {
		fprintf(stderr, "%s: AVR '%s' not known\n", argv[0], f.mmcu);
		exit(1);
	}
	printf("firmware %s f=%d mmcu=%s\n", fname, (int) f.frequency, f.mmcu);

	avr_init(avr);
	avr_load_firmware(avr, &f);

    /* External parts connections */
    uart_pty_init(avr, &uart_pty);
	uart_pty_connect(&uart_pty, '0');
	uint32_t tHigh, tLow;
    RPMtoPeriod(6000, &tHigh, &tLow);
    pulse_input_init(avr, &pulse_input_engine, "Engine", tHigh, tLow);
    SpeedtoPeriod(50, &tHigh, &tLow);
    pulse_input_init(avr, &pulse_input_wheel, "Wheel", tHigh, tLow);
	avr_connect_irq(pulse_input_engine.irq + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
	avr_connect_irq(pulse_input_wheel.irq  + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3));
    avr_irq_t * i_pwm = avr_io_getirq(avr, AVR_IOCTL_TIMER_GETIRQ('0'), TIMER_IRQ_OUT_PWM1);
	avr_irq_register_notify(i_pwm, pwm_changed_hook, NULL);	
	
    /* VCD files */
    avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 100 /* usec */);
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5),
			1 /* bits */, "LED");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 6),
			1 /* bits */, "Image");

	avr_vcd_add_signal(&vcd_file, pulse_input_engine.irq + IRQ_PULSE_OUT, 1, "Pulse_engine");
	avr_vcd_add_signal(&vcd_file, pulse_input_wheel.irq  + IRQ_PULSE_OUT, 1, "Pulse_Wheel");
    avr_vcd_add_signal(&vcd_file, i_pwm, 8 /* bits */, "PWM" );
	avr_vcd_start(&vcd_file);

	pthread_t run;
	pthread_create(&run, NULL, avr_run_thread, NULL);
    /*int i;
    for(i=0;i<10000000;i++)
    {
        avr_run(avr);
    }*/

    sleep(20);

    return 0;
}
