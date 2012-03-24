#define F_CPU 16000000

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define IR_READY 224
#define IR_SHOOT 225

#define STATUS_PIN 32 // Status LED is on PB5
#define MOSFET_PIN 2  // MOSFET (IR Emitter) is driven on PB1
#define IR_DETECT_PIN 1 // IR Detect Pin (Active Low on hit) on PB0

#define HIT_PIN 128   // Hit LEDs are driven from PD7
#define BUZZER_PIN 8  // Piezo buzzer driven on PD3

#define TRIGGER_PIN 4 // Trigger pin is PD2/INT0

volatile uint8_t ir_data = 0;
volatile uint8_t ir_on = 1;

// Define IR signals here
ISR(TIMER1_COMPA_vect)
{
	if(ir_data-- && !ir_on)
	{
		TCCR1A |= _BV(COM1A0);
		ir_on = 1;
	} else {
		TCCR1A &= ~_BV(COM1A0);
	}
}

// Startup initialization
void setup()
{
	// Setup data direction registers
	// 1 is output, 0 is input
	DDRD |= BUZZER_PIN;
	DDRD |= HIT_PIN;
	DDRB |= STATUS_PIN;

	// Turn on the power LED
	PORTB |= STATUS_PIN;

	// Turn off hit LEDs
	PORTD |= HIT_PIN;

	// Enable IR output
	DDRB |= MOSFET_PIN;

	// Set pull-up resistor on IR detector
	PORTB |= IR_DETECT_PIN;

	// Setup 38khz timer
	// Timer1 with /8 prescaler
	TCCR1A = _BV(WGM10) | _BV(WGM11);
	TCCR1B = _BV(CS11) | _BV(WGM12) | _BV(WGM13);
	
	// Interrupt at match
	TIMSK1 = _BV(OCIE1A);

	// 38Khz timer match
	// This will produce a 50% duty cycle signal at 38Khz
	OCR1A = 26;

	// Enable global interrupts
	sei();
}

int main()
{
	setup();
	while(1)
	{
	}

	return 0;
}
