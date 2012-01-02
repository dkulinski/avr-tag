#define F_CPU 16000000

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t note_length = 0xff;
volatile uint8_t current_note_length = 0;
volatile uint8_t current_note = 0;

// Predefine sounds
// x[0] = number of notes
// x[1..(x[0]-1)] = notes
uint8_t fire[7] = {6,100,128,100,90,90,90};
uint8_t beeper[129] = {8,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// Sound pointer
uint8_t * sound1; 
#define STATUS_PIN 32 // Status LED is on PB5
#define MOSFET_PIN 2  // MOSFET is driven on PB1

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
		current_note = 1;
	}
}

ISR(INT0_vect)
{
}

ISR(TIMER2_COMPA_vect)
{
	if(current_note > 0)
	{
		TCCR2A |= _BV(COM2B1);
		if(current_note >= sound1[0])
		{
			current_note = 0;
		} else {
			// Always keep a 50% duty cycle to 
			// maintain volume
			OCR2A = sound1[current_note];
			OCR2B = sound1[current_note]/2;
			if(current_note_length >= note_length)
			{
				current_note_length = 0;
				current_note++;
			} else {
				current_note_length++;
			}
		} 
	} else {
		TCCR2A &= ~_BV(COM2B1);
	}
}

int main()
{
	// Setup data direction registers
	DDRD = 0b00001000;
	DDRB |= STATUS_PIN;

	// Turn on the power light
	PORTB |= STATUS_PIN;
	
	// Set piezo timer up
	// Timer 3 with a 256 prescaler
	TCCR2A = _BV(WGM20) | _BV(WGM21);
	TCCR2B = _BV(CS20) | _BV(WGM22);
	//TCCR2B = _BV(CS22) | _BV(CS21) | _BV(WGM22);

	// Interrupt enable mask
	TIMSK2 = _BV(OCIE2A);

	// Enable global interrupt flag
	sei();

	while(1) 
	{
		if(current_state == GAME_WAIT)
		{
			beep();
		}
	}
	return 0;
}

