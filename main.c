#define F_CPU 16000000L	/* Clock Frequency = 16MHz */
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t led = 0x00;

// Interrupt to handle changes on pin 14 (PCINT0)
ISR(PCINT0_vect)
{
	led = 0xff;
}

int main() 
{
	// All port D pins are inputs except the hit status LEDs
  DDRD=0b10000000;
	// Operational LED pin set to output
	DDRB=0b00100000;
	// Turn on power to operation LED and set pull-up resistor for IR signal in
	PORTB=0b00100001;

	// Start status LEDs in off mode
	PORTD=0b10000000;

	// Pin Change Interrup Control Register - Enable the lowest Pin Change Register
	PCICR = _BV(PCIE0);
	// Pint Change Mask 0 - Enable PCINT0
	PCMSK0 = _BV(PCINT0);

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
