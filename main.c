#define F_CPU 16000000L	/* Clock Frequency = 16MHz */
#define NOTE_LENGTH 24;
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t led = 0x00;

uint8_t fire[7] = {128,64,128,64,32,32,32};
uint8_t num_notes = 7;

uint8_t current_note = 0;
uint8_t current_note_length = 0;

uint8_t sound_active = 0;

ISR(INT0_vect)
{
	if(!sound_active)
		sound_active = 1;
}

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
	// All port D pins are inputs except the hit status LEDs and Buzzer
  DDRD=0b10001100;
	// Operational LED pin set to output
	DDRB=0b00100010;
	DDRB &= ~_BV(DDB0);

	// Turn on power to operation LED and set pull-up resistor for IR signal in
	PORTB=0b00100001;

	// Start status LEDs in off mode
	// Set the internal pull-up on D2
	PORTD=0b10000100;

	// Enable timer 0 with 8 prescaler
	// Count for 26 microseconds
	TCCR0A = _BV(WGM00) | _BV(WGM01);
	TCCR0B = _BV(WGM02) | _BV(CS01);
	OCR0A = 52;
	
	// Enable Output compare match
	TIMSK0 = _BV(OCIE0A);

	// Enable timer 1 with 8 prescaler
	TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
  OCR1A = 0x0034;

	// ENable output compare match
	//TIMSK1 = _BV(OCIE1A);

	// Enable timer 2 with 32 prescaler
	// Output disabled
	TCCR2A = _BV(WGM20) | _BV(WGM21) | _BV(COM2B1);
	TCCR2B = _BV(CS21) | _BV(CS20) | _BV(WGM22);
	OCR2A = 128;
	OCR2B = 16;

	// Enable output compare match
	TIMSK2 = _BV(OCIE2B);

	// Enable interrupt on INT0 (pin 4)
//	EIMSK = _BV(INT0);

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
