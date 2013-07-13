#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "analog_input.h"


static const char * irq_names[IRQ_ANALOG_INPUT_COUNT] = {
		[IRQ_ANALOG_INPUT_VALUE] 		 = "<analog_input.value",
};

void
analog_input_init(
		struct avr_t * avr,
		analog_input_t *p,
		const float value,
		const char *name)
{
	char *pName = &(p->name[0]);
	strncpy(pName, name, 256);
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_ANALOG_INPUT_COUNT, irq_names);
	p->value = value;
	avr_raise_irq(p->irq + IRQ_ANALOG_INPUT_VALUE, value);
}

void
analog_input_set_value(
		analog_input_t *p,
		const float value)
{
	p->value = value;
	avr_raise_irq(p->irq + IRQ_ANALOG_INPUT_VALUE, value);
}
