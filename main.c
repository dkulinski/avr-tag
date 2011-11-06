#define F_CPU 16000000L	/* Clock Frequency = 16MHz */
#define NOTE_LENGTH 1250L
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t led = 0x00;

uint8_t fire[7] = {128,100,128,100,90,90,90};
uint8_t num_notes = 7;

uint8_t current_note = 0;
uint16_t current_note_length = 0;

uint8_t sound_active = 0;
uint8_t fire_led_active = 0;

// IR reception variables
uint8_t last_ir_in = 0;
uint8_t curr_bit = 0;
uint8_t header_received = 0;
uint8_t data_wait = 0;
uint16_t packet = 0x0000;

enum states { GAME_IN_PROGRESS, GAME_OVER };

// Trigger pull interrupt
ISR(INT0_vect)
{
	// sound_active = 1;
	// fire_led_active = 1;
	// OCR2A = fire[0];
}

// Interrupt to handle changes on pin 14 (PCINT0)
ISR(TIMER0_COMPA_vect)
{
	last_ir_in++;
	if( ~PINB & _BV(PB0) )
	{
		if(data_wait)
		{
			if(last_ir_in < 7)
			{
				return;
			} else {
				data_wait = 0;
				last_ir_in = 0;
			}
		}
		if(header_received == 0)
		{
			if(last_ir_in > 13 || last_ir_in < 17)
			{
				curr_bit++;
				last_ir_in = 0;
				if(curr_bit == 15)
				{
					data_wait = 1;
					header_received = 1;
					curr_bit = 0;
					led = 0x4f;
				}
			}
		} else {
			
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	if(fire_led_active)
	{
		TCCR1A |= _BV(COM1A1);
	} else {
		TCCR1A &= ~_BV(COM1A1);
	}
}

ISR(TIMER2_COMPA_vect)
{
	if(sound_active == 1)
	{
		TCCR2A |= _BV(COM2B1);
		if(current_note_length >= NOTE_LENGTH)
		{
			current_note_length = 0;
			if(current_note >= num_notes)
			{
				current_note = 0;
				sound_active = 0;
				TCCR2A &= ~_BV(COM2B1);
			} else {
				current_note++;
				OCR2A = fire[current_note];
			}
		} else {
			current_note_length++;
		}
	}
}

int main() 
{
	// All port D pins are inputs except the hit status LEDs and Buzzer
  DDRD=0b10001000;
	// Operational LED pin set to output
	DDRB=0b00100010;
	//DDRB &= ~_BV(DDB0);

	// Turn on power to operation LED and set pull-up resistor for IR signal in
	PORTB=0b00100001;

	// Start status LEDs in off mode
	PORTD=0b10000000;

	// Enable timer 0 with 8 prescaler
	// Count for 26 microseconds
	TCCR0A = _BV(WGM00) | _BV(WGM01);
	TCCR0B = _BV(WGM02) | _BV(CS01);
	OCR0A = 52;
	
	// Enable Output compare match
	TIMSK0 = _BV(OCIE0A);

	// Enable timer 1 with 8 prescaler
	// TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
	TCCR1A = _BV(WGM10) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
  OCR1A = 0x0034;

	// Enable output compare match
	TIMSK1 = _BV(OCIE1A);

	// Enable timer 2 with 32 prescaler
	// Output disabled
	// TCCR2A = _BV(WGM20) | _BV(WGM21) | _BV(COM2B1);
	TCCR2A = _BV(WGM20) | _BV(WGM21);
	TCCR2B = _BV(CS21) | _BV(CS20) | _BV(WGM22);
	OCR2A = 128;
	OCR2B = 16;

	// Enable output compare match
	TIMSK2 = _BV(OCIE2A);

	// Enable interrupt on INT0 (pin 4)
	EICRA = _BV(ISC01);
	EIMSK = _BV(INT0);

	// Enable interrupts according to registers
	sei();

	enum states curr_state = GAME_IN_PROGRESS;

  while(1)
  {
		if(curr_state == GAME_IN_PROGRESS)
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
  }
  return 0;
}
