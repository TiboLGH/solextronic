#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "sim_time.h"
#include "timing_analyzer.h"


static const char * irq_names[IRQ_TIMING_ANALYZER_COUNT] = {
		[IRQ_TIMING_ANALYZER_IN] = "1<timing_analyzer.in",
		[IRQ_TIMING_ANALYZER_REF_IN] = "1<timing_analyzer.ref_in",
};

static volatile uint32_t last_ref_in_ts; 
static volatile uint32_t last_in_rising_ts; 
static volatile uint32_t last_in_falling_ts; 
static volatile uint32_t analyzed_cycles; 

/*
 * called when pin to analyze toggle
 */
static void timing_analyzer_in_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	timing_analyzer_t * p = (timing_analyzer_t*)param;
	uint32_t ts = p->avr->cycle; 
	// get timestamp and direction
	if (irq->value && value) {	// rising edge
		p->result.period 		= avr_cycles_to_usec(p->avr, ts - last_in_rising_ts);
		p->result.rising_offset = avr_cycles_to_usec(p->avr, ts - last_ref_in_ts);
		last_in_rising_ts = ts;
		analyzed_cycles++;
	}
	else if(irq->value && !value) {	// falling edge
		p->result.falling_offset = avr_cycles_to_usec(p->avr, ts - last_ref_in_ts);
		p->result.high_duration  = avr_cycles_to_usec(p->avr, ts - last_in_rising_ts);
		last_in_falling_ts = ts;
	}
}

/*
 * called when ref pin toggle
 */
static void timing_analyzer_ref_in_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
	timing_analyzer_t * p = (timing_analyzer_t*)param;
	uint32_t ts = p->avr->cycle; 
	// get timestamp and direction
	if (irq->value && value) {	// rising edge
		last_ref_in_ts = ts;
	}
}

void
timing_analyzer_init(
		struct avr_t * avr,
		timing_analyzer_t *p,
		const char *name)
{
	p->avr = avr;
	char *pName = &(p->name[0]);
	strncpy(pName, name, 256);
	p->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_TIMING_ANALYZER_COUNT, irq_names);
	avr_irq_register_notify(p->irq + IRQ_TIMING_ANALYZER_IN, timing_analyzer_in_hook, p);
	avr_irq_register_notify(p->irq + IRQ_TIMING_ANALYZER_REF_IN, timing_analyzer_ref_in_hook, p);
	
	p->result.rising_offset 	= 0;
	p->result.falling_offset 	= 0;
	p->result.period 			= 0;
	p->result.high_duration 	= 0;
	analyzed_cycles 			= 0;
}

/*
 * Called by test bench to get results
 * return the number of cycle analyzed (note only the last one metrics are reported)
 * once function is called, number of analyzed cycles is reset
 */
int
timing_analyzer_result(
	timing_analyzer_t *p,
	timing_analyzer_result_t* result)
{
	uint32_t cycles = analyzed_cycles;
	analyzed_cycles = 0;
	
	memcpy(result, &(p->result), sizeof(timing_analyzer_result_t));
	return cycles;
}
