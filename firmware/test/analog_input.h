
#ifndef __ANALOG_INPUT_H__
#define __ANALOG_INPUT_H__

#include "sim_irq.h"

/*
 * This part is a anlog input driver
 */
enum {
	IRQ_ANALOG_INPUT_VALUE = 0,	
	IRQ_ANALOG_INPUT_COUNT
};

typedef struct analog_input_t {
	avr_irq_t *	irq;		// irq list
    char      name[256];
	float	  value;		// value in v
} analog_input_t;

void
analog_input_init(
		struct avr_t * avr,
		analog_input_t *p,
		const float value,
		const char *name);
void
analog_input_set_value(
		analog_input_t *p,
		const float value);

#endif
