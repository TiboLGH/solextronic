#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_DDR		DDRB
#define LED_PORT	PORTB
#define LED			PB5
#define LED_PIN		PINB

#define BAUD 	57600
#define MAXSTR 	10

volatile unsigned char rReady = 0;
volatile unsigned char rIndex = 0;
volatile unsigned char rBuffer[MAXSTR+1];


void InitUart(void);
void getsInt(void);
static void putchr(char c);


int main(void)
{
	LED_DDR |= _BV(LED);
	
	InitUart();

	sei();

	while(1)
	{
		/*LED_PIN |= _BV(LED);
		_delay_ms(50);
		LED_PIN |= _BV(LED);
		_delay_ms(1000);*/
		if(rReady)
		{
			if(rIndex == 1)
			{
				LED_PIN |= _BV(LED);
			}
			getsInt();
		}
	}

	return(0);
}


void InitUart(void)
{
	/* Macros is setbaud.h commpute the regs values */
#include <util/setbaud.h>
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;
   #if USE_2X
   UCSR0A |= (1 << U2X0);
   #else
   UCSR0A &= ~(1 << U2X0);
   #endif
   UCSR0B = (1 << RXEN0) | (1 << TXEN0);
   UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
   UCSR0B = (1 << RXCIE0);
   return;
}

void getsInt(void)
{
	rReady = 0;
	rIndex = 0;
	UCSR0B |= _BV(RXCIE0);
}

ISR(USART_RX_vect)
{
	char c;
	c = UDR0;
	if(bit_is_clear(UCSR0A, FE0))
	{
		if(c != '\r')
		{
			putchr(c);
			if(rIndex < MAXSTR)
			{
				rBuffer[rIndex++] = c;
			}
		}else{
			putchr('\r');
			putchr('\n');
			rReady = 1;
			UCSR0B &= ~_BV(RXCIE0);
		}
	}
}

static void putchr(char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}
