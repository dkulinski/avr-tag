#define F_CPU 16000000

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define IR_READY 224
#define IR_SHOOT 225

volatile uint8_t note_length = 0x0f;
volatile uint8_t current_note_length = 0;
volatile uint8_t current_note = 0;

volatile uint8_t ir_count = 0;
volatile uint8_t ir_hit = 0;
volatile uint8_t ir_out = 0;
volatile uint8_t ir_data = 0;
volatile uint8_t ir_receive = 0;

uint8_t ir_in = 0;

//volatile uint8_t trigger_down = 0;

volatile int trigger_count = 0;

// Predefine sounds
// x[0] = number of notes
// x[1] = note_length
// x[2..(x[0]-1)] = notes
uint8_t fire[10] = {8,32,12,12,12,16,16,16,18,18};
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
enum states { GAME_RUNNING, GAME_OVER, GAME_WAIT, GAME_SETTINGS, GAME_SETUP };

enum states current_state = GAME_WAIT;

void send_data(uint8_t data)
{
	if(!ir_data)
	{
		ir_out = data;
		ir_data = 2;
	}
}

void sound(uint8_t * melody, uint8_t priority)
{
	if(current_note < 1)
	{
		sound1 = melody;
		current_note = 2;
	} 
	if(priority == 1)
	{
		sound1 = melody;
		current_note = 2;
	}
}

// 38Khz loop check
ISR(TIMER1_COMPA_vect)
{
	// Turn on the IR LED if there is data in the 
	// most significant bit otherwise turn it off
	if((ir_out & 0xe0) && ir_data--)
	{
		PORTB |= MOSFET_PIN;
	} else {
	  PORTB &= ~MOSFET_PIN;
	}

	if(ir_out && !ir_data)
		ir_out = ir_out << 1;


	// Count pin down count
	if(PIND & TRIGGER_PIN)
	{
		trigger_count++;
	} else {
		trigger_count = 0;
	}
	
	// Count each cycle and set ir hit if data received
	ir_count++;
	ir_hit = ~PINB & IR_DETECT_PIN;

	if(ir_hit)
		ir_receive += 1;
	ir_receive = ir_receive << 1;

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
	int ir_timeout = 0;

	// Setup data direction registers
	// 1 is output, 0 is input
	DDRD |= BUZZER_PIN;
	DDRD |= HIT_PIN;
	DDRB |= STATUS_PIN;

	// Turn off hit LEDs
	PORTD |= HIT_PIN;

	// Turn on the power light
	PORTB |= STATUS_PIN;

	// Enable IR output
	DDRB |= MOSFET_PIN;

	// Set pull-up resistor on IR detector
	PORTB |= IR_DETECT_PIN;
	
	// Set 38Khz timer up
	// Timer0 with 8 prescaler
	TCCR1A = _BV(WGM10) | _BV(WGM11);
	TCCR1B = _BV(CS11) | _BV(WGM12) | _BV(WGM13);

	TIMSK1 = _BV(OCIE1A);

	// 38Khz timer interrupt
	OCR1A = 53;

	// Set piezo timer up
	// Timer 3 with a 256 prescaler
	TCCR2A = _BV(WGM20) | _BV(WGM21);
	//TCCR2B = _BV(CS21) | _BV(WGM22);
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(WGM22);

	// Interrupt enable mask
	TIMSK2 = _BV(OCIE2A);

	// Interrupt enable on INT0 (pin 4)
	//EICRA = _BV(ISC01);
	//EIMSK = _BV(INT0);

	// Enable global interrupt flag
	sei();

	while(1) 
	{

		if(current_state == GAME_WAIT)
		{
			sound(beeper,0);
			if(trigger_count > 16000)
			{
				sound(fire,1);
				current_state =  GAME_SETUP;
				ir_timeout = 0;
			}
		}

		if(current_state == GAME_SETUP)
		{
			if(ir_count == 255)
				ir_timeout++;
			if(ir_timeout == 255)
			{
				send_data(IR_READY);
				ir_timeout = 0;
			}
			if(ir_hit)
			{
				PORTD &= ~HIT_PIN;
				ir_receive += 1;
			} else {
				PORTD |= HIT_PIN;
			}

			if(ir_receive == IR_READY)
			{
				//current_state = GAME_RUNNING;
				ir_receive =0;
				sound(beeper, 1);
			}
				
		}
	}
	return 0;
}

