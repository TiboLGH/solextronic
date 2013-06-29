/*
	charlcd.c

	Copyright Luki <humbell@ethz.ch>
	Copyright 2011 Michel Pollet <buserror@gmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_timer.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_vcd_file.h"
#include "sim_irq.h"
#include "sim_time.h"
#include "uart_pty.h"

//#include <pthread.h>

avr_t * avr = NULL;
avr_vcd_t vcd_file;
uart_pty_t uart_pty;

enum {
    IRQ_PULSE_OUT = 0,
    IRQ_PULSE_QTY
};

typedef struct pulse_input_t {
    avr_irq_t * irq;
    struct avr_t * avr;
    uint8_t value;
    uint32_t high;
    uint32_t low;
} pulse_input_t;

pulse_input_t pulse_input;

static avr_cycle_count_t switch_auto(struct avr_t * avr, avr_cycle_count_t when, void * param)
{
	pulse_input_t * b = (pulse_input_t *) param;
	b->value = !b->value;
	avr_raise_irq(b->irq + IRQ_PULSE_OUT, b->value);
    if(b->value) 
        return when + avr_usec_to_cycles(avr, b->high);
    else
        return when + avr_usec_to_cycles(avr, b->low);
}

static const char * name = ">pulse_input";

void pulse_input_init(avr_t *avr, pulse_input_t *b, const uint32_t tHigh, uint32_t tLow)
{
	b->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_PULSE_QTY, &name);
	b->avr = avr;
	b->value = 0;
    b->low = tLow;
    b->high = tHigh;
	avr_cycle_timer_register_usec(avr, b->low, switch_auto, b);
	printf("pulse_input_init period %duS, duty cycle %.1f%%\n", tHigh+tLow, 100*(float)tHigh/(tHigh+tLow));
}
volatile uint8_t	display_pwm = 0;
void pwm_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	display_pwm = value;
}

int main(int argc, char *argv[])
{
	elf_firmware_t f;
	const char * fname = "test.elf";
	elf_read_firmware(fname, &f);
    
	printf("firmware %s f=%d mmcu=%s\n", fname, (int) f.frequency, f.mmcu);
    strcpy(f.mmcu, "atmega328");
    f.frequency = 16000000;
	avr = avr_make_mcu_by_name(f.mmcu);
	if (!avr) {
		fprintf(stderr, "%s: AVR '%s' not known\n", argv[0], f.mmcu);
		exit(1);
	}

	avr_init(avr);
	avr_load_firmware(avr, &f);
	pulse_input_init(avr, &pulse_input, 100, 9900);
	avr_connect_irq(pulse_input.irq + IRQ_PULSE_OUT, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
    avr_irq_t * i_pwm = avr_io_getirq(avr, AVR_IOCTL_TIMER_GETIRQ('0'), TIMER_IRQ_OUT_PWM1);
	avr_irq_register_notify(i_pwm, pwm_changed_hook, NULL);	
	avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 100 /* usec */);
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5),
			1 /* bits */, "LED");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 6),
			1 /* bits */, "Image");

	avr_vcd_add_signal(&vcd_file, pulse_input.irq + IRQ_PULSE_OUT, 1, "pulse_input");
    avr_vcd_add_signal(&vcd_file, i_pwm, 8 /* bits */, "PWM" );
	avr_vcd_start(&vcd_file);

	//pthread_t run;
	//pthread_create(&run, NULL, avr_run_thread, NULL);
    int i;
    for(i=0;i<10000000;i++)
    {
        avr_run(avr);
    }

    return 0;
}
