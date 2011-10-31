#define F_CPU 16000000UL

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

void setup()
{
	// Set output pin
	DDRD=0b01000000;
	// Set up timer 0.  Clock prescaled by 8.  Toggle pin 12
	TCCR0A = _BV(WGM00) | _BV(WGM01) | _BV(COM0A0);
	TCCR0B = _BV(CS01);

	// Top value to match
	OCR0A = 52;
	
}

int main()
{
	setup();

	while(1)
	{
		TCCR0B = 10;
		_delay_us(260);
		TCCR0B = 2;
		_delay_ms(1000);
	}

	return 0;
}
