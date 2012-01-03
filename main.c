#define F_CPU 16000000

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t note_length = 0x0f;
volatile uint8_t current_note_length = 0;
volatile uint8_t current_note = 0;

volatile uint8_t ir_count = 0;
volatile uint8_t ir_hit = 0;
volatile uint8_t ir_out = 0;

// Predefine sounds
// x[0] = number of notes
// x[1] = note_length
// x[2..(x[0]-1)] = notes
uint8_t fire[8] = {6,15,100,128,100,90,90,90};
uint8_t beeper[66] = {64,255,
32,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
// Sound pointer
volatile uint8_t * sound1; 
#define STATUS_PIN 32 // Status LED is on PB5
#define MOSFET_PIN 2  // MOSFET (IR Emitter) is driven on PB1
#define IR_DETECT_PIN 1 // IR Detect Pin (Active Low on hit) on PB0

#define HIT_PIN 128   // Hit LEDs are driven from PD7
#define BUZZER_PIN 8  // Piezo buzzer driven on PD3

#define TRIGGER_PIN 4 // Trigger pin is PD2/INT0

// States for the game
// 
// The game can be running, over,waiting to start,
// or in game settings
//
// When the game is running check IR sensor, fire, play appropriate
// sounds.  
// When the game is in the wait state, player one will pull trigger
// to start the game
// In the settings menu (to be done) the player can change settings
// of the gun
enum states { GAME_RUNNING, GAME_OVER, GAME_WAIT, GAME_SETTINGS };

enum states current_state = GAME_WAIT;

void beep()
{
	if(current_note < 1)
	{
		sound1 = beeper;
		current_note = 2;
	}
}

ISR(INT0_vect)
{
	ir_out = 0xff;
}

// 38KHz loop check
ISR(TIMER0_COMPA_vect)
{
	// Count each cycle and set ir hit if data received
	ir_count++;
	ir_hit = PINB & IR_DETECT_PIN;
	if(ir_out != 0)
	{
		TCCR0A |= _BV(COM0A1);
		ir_out = ir_out << 1;
	} else {
		TCCR0A &= ~_BV(COM0A1);
	}
}

// Piezo Buzzer
ISR(TIMER2_COMPA_vect)
{
	if(current_note > 0)
	{
		note_length = sound1[1];
		//TCCR2A |= _BV(COM2B1);
		if(current_note > sound1[0])
		{
			current_note = 0;
			TCCR2A &= ~_BV(COM2B1);
		} else {
			// Always keep a 50% duty cycle to 
			// maintain volume
			OCR2A = sound1[current_note];
			OCR2B = sound1[current_note]/2;

			if(sound1[current_note] == 0)
			{
				TCCR2A &= ~_BV(COM2B1);
			} else {
				TCCR2A |= _BV(COM2B1);
			}

			if(current_note_length >= note_length)
			{
				current_note_length = 0;
				current_note++;
			} else {
				current_note_length++;
			}
		} 
	}
}

int main()
{
	// Setup data direction registers
	// 1 is output, 0 is input
	DDRD = 0b00001000;
	DDRB |= STATUS_PIN;

	// Turn on the power light
	PORTB |= STATUS_PIN;

	// Enable IR output
	DDRB |= MOSFET_PIN;
	
	// Set 38Khz timer up
	// Timer0 with 8 prescaler
	TCCR0A = _BV(WGM00) | _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(WGM02);
	TIMSK0 = _BV(OCIE0A);

	// 38Khz timer interrupt
	OCR0A = 53;

	// Set piezo timer up
	// Timer 3 with a 256 prescaler
	TCCR2A = _BV(WGM20) | _BV(WGM21);
	//TCCR2B = _BV(CS21) | _BV(WGM22);
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(WGM22);

	// Interrupt enable mask
	TIMSK2 = _BV(OCIE2A);

	// Interrupt enable on INT0 (pin 4)
	EICRA = _BV(ISC01);
	EIMSK = _BV(INT0);

	// Enable global interrupt flag
	sei();

	while(1) 
	{
		if(current_state == GAME_WAIT)
		{
			//beep();
		}
	}
	return 0;
}

