
#ifndef __TIMING_ANALYZER_H__
#define __TIMING_ANALYZER_H__

#include "sim_irq.h"

/*
 * This part is a timing analyzer. Based on reference signal rising edge, it
 * compute several parameters of an output signal (supposed sync with reference).
 */
enum {
	IRQ_TIMING_ANALYZER_IN = 0,	
	IRQ_TIMING_ANALYZER_REF_IN,	
	IRQ_TIMING_ANALYZER_COUNT
};

typedef struct timing_analyzer_result_t {
	uint32_t	rising_offset;		// irq list
	uint32_t	falling_offset;	// value "on the pins"
	uint32_t 	period;				// value shifted in
	uint32_t 	high_duration;	// value shifted in
} timing_analyzer_result_t;

typedef struct timing_analyzer_t {
	avr_irq_t *	irq;		// irq list
	struct avr_t * avr;		// avr instance for current time
	char		name[256];	// analyzer name, to use multiple instances
	timing_analyzer_result_t  	result;		// analysis result
} timing_analyzer_t;



void
timing_analyzer_init(
		struct avr_t * avr,
		timing_analyzer_t *p,
		const char *name);

int
timing_analyzer_result(
	timing_analyzer_t *p,
	timing_analyzer_result_t* result);
		
		
#endif
