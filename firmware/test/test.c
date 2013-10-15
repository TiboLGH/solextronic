#include <avr/io.h>
#include <util/delay.h>

/*
  Atmega8 ports & Arduino pins:
  port B -> digital pins 8-13
  port C -> analog input pins 0-5
  port D -> digital pins 0-7

  http://arduino.cc/en/Hacking/PinMapping


  Connect a LED between pin 13 and GND.
 */


void delay()
{
  unsigned char counter = 0;
  while (counter != 50)
  {
    /* wait (30000 x 4) cycles = wait 120000 cycles */
    _delay_loop_2(3000);
    counter++;
  }
}

int main(void)
{
  /* Initialization, set PB5 (arduino digital pin 13) as output */
  DDRB |= (1<<PB5) | (1<<PB6);
  DDRD &= ~(1 << DDD2);
  unsigned char inputState = 0;
  unsigned char inputLatch = 0;
  // Set PD5/OC0B as output
  DDRD |= (1 << DDD5);
  // Fast PWM, prescaler 1 (~60kHz), output on OC0B
  TCCR0A = (0 << COM0A1) | (0 << COM0A0) |
      (1 << COM0B1) | (0 << COM0B0) |
      (1 << WGM01)  | (1 << WGM00);
  TCCR0B = (0 << FOC0A)  | (0 << FOC0B) |
      (0 << WGM02)  | (0	<< CS02)  |
      (1 << CS01)   | (0 << CS00); 

  OCR0B = 0; // off state

  while (1) 
  {
    //PORTB |= (1<<PB5);  //arduino digital pin 5 -> 5V
    //delay();
    //PORTB &= ~(1<<PB5);  //arduino digital pin 5 -> GND   
    //delay();
    inputLatch = PIND & (1<<PIND2);
    // rising edge
    if(inputLatch && !inputState)
    {
        OCR0B += 64; 
        PINB |= _BV(PB6);
    } 
    inputState = inputLatch;
  }
  return 0;
}
