#define F_CPU 16000000L	/* Clock Frequency = 16MHz */
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t led = 0x00;

// Interrupt to handle changes on pin 14 (PCINT0)
ISR(TIMER0_COMPA_vect)
{
	if( ~PINB & _BV(PB0) )
	{
		led = 0x2f;
	}
}

int main() 
{
	// All port D pins are inputs except the hit status LEDs
  DDRD=0b10000000;
	// Operational LED pin set to output
	DDRB=0b00100000;
	DDRB &= ~_BV(DDB0);

	// Turn on power to operation LED and set pull-up resistor for IR signal in
	PORTB=0b00100001;

	// Start status LEDs in off mode
	PORTD=0b10000000;

	// Enable timer 0 with 8 bit prescaler
	// Count for 26 microseconds
	TCCR0A = _BV(WGM00) | _BV(WGM01);
	TCCR0B = _BV(WGM02) | _BV(CS01);
	OCR0A = 52;

	// Enable Output compare match
	TIMSK0 = _BV(OCIE0A);

	// Enable interrupts according to registers
	sei();

  while(1)
  {

		if(led > 0)
		{
			led--;
			PORTD=0b00000000;
		} else 
		{
			PORTD=0b10000000;	
		}
  }
  return 0;
}
