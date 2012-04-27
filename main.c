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

#define HEADER_LENGTH 128 // Header bits

#define LCD_DATA 1 // LCD data pin PC0
#define LCD_CHARACTER 0x10 // LCD clock pin PC4
#define LCD_ENABLE 0x20 // LCD enable pin PC5

enum state { INIT, READY, RUNNING };

volatile uint8_t ir_length = 0;
volatile uint8_t ir_high = 0;
volatile uint8_t ir_detect = 1;
volatile uint8_t trigger_down = 0;
volatile uint16_t cycle_count = 0;
volatile uint16_t note_length = 0;
volatile uint8_t shift_num = 0;
volatile uint8_t shift_data = 0;
volatile uint8_t shift_ready = 0;

// Define IR signals here
//
//

// 76KHz cycle
ISR(TIMER1_COMPA_vect)
{
	cycle_count++;
	note_length--;
	
	if(ir_length && ir_high)
	{
		ir_length--;
		TCCR1A |= _BV(COM1A0);
	} else {
		TCCR1A &= ~_BV(COM1A0);
	}

	// IR detector puts out an active low on signal
	// So negate and AND to get the right logic
	ir_detect = ~PINB & IR_DETECT_PIN;

	// Set trigger status
	trigger_down = PIND & TRIGGER_PIN;
}

void lcd_send(uint8_t data, uint8_t character)
{
	PORTC = 0;

	if(character)
		PORTC |= LCD_CHARACTER;
	else
		PORTC &= ~LCD_CHARACTER;
	
	PORTC |= (data & 0x0f); // Send lower 4 bits

	PORTC |= LCD_ENABLE;
	_delay_us(450);
	PORTC &= ~LCD_ENABLE;
}

// Send data to LCD 4 bits at a time
void write_byte(uint8_t data, uint8_t command)
{
	lcd_send((data >> 4), !command);
	_delay_us(200);
	lcd_send(data, !command);
	_delay_us(200);
}

void write_bytes(const char const * str, int length)
{
	int i;
	for(i=0; i < length; i++)
	{
		write_byte(str[i],0);
	}
}

void lcd_clear()
{
	write_byte(0x01, 1);
}


// Startup initialization
void setup()
{
	// Setup data direction registers
	// 1 is output, 0 is input
	DDRD |= BUZZER_PIN;
	DDRD |= HIT_PIN;
	DDRB |= STATUS_PIN;

	// Enable output for LCD (shift register)
	DDRC |= 0x3f; // Enable LCD ports

	PORTC = 0x00; // Turn all ports off


	// Turn off hit LEDs
	PORTD |= HIT_PIN;

	// Enable IR output
	DDRB |= MOSFET_PIN;

	// Set pull-up resistor on IR detector
	PORTB |= IR_DETECT_PIN;

	// Setup 76KHz timer
	// Timer1 with /8 prescaler
	TCCR1A = _BV(WGM10) | _BV(WGM11);
	TCCR1B = _BV(CS11) | _BV(WGM12) | _BV(WGM13);
	
	// Interrupt at match
	TIMSK1 = _BV(OCIE1A);

	// 38Khz timer match
	// This will produce a 50% duty cycle signal at 38Khz
	OCR1A = 26;

	// Setup timer for piezo buzzer
	TCCR2A = _BV(WGM20) | _BV(WGM21);
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(WGM22); 

	// LCD setup
	_delay_ms(20);
	lcd_send(0x03,0);
	_delay_ms(5);
	lcd_send(0x03,0);
	_delay_us(200);
	lcd_send(0x03,0);
	_delay_us(200);
	lcd_send(0x02,0);
	_delay_ms(5);
	//write_byte(0x28,0);
	lcd_send(0x02,0);
	_delay_us(200);
	lcd_send(0x08,0);
	_delay_us(200);
	//write_byte(0x08,0);
	lcd_send(0x00,0);
	_delay_us(200);
	lcd_send(0x08,0);
	_delay_us(200);
	//write_byte(0x01,0);
	lcd_send(0x00,0);
	_delay_us(200);
	lcd_send(0x01,0);
	_delay_us(200);
	_delay_ms(5);
	//write_byte(0x06,0);
	lcd_send(0x00,0);
	_delay_us(200);
	lcd_send(0x06,0);
	_delay_us(200);
	//write_byte(0x0C,0);
	lcd_send(0x00,0);
	_delay_us(200);
	lcd_send(0x0C,0);
	_delay_us(200);
	_delay_ms(5);

	//write_byte(0x80,0);
	lcd_send(0x08,0);
	_delay_us(200);
	lcd_send(0x00,0);
	_delay_us(200);
	_delay_ms(5);

	
	// Enable global interrupts
	sei();

}

void play_note(char note, int length)
{
	// If the length is a non-zero value turn on PWM
	// otherwise disable it.
	if(length)
	{
		OCR2A = note;
		OCR2B = note/2;
		TCCR2A |= _BV(COM2B1);
	} else {
		TCCR2A &= ~_BV(COM2B1);
	}
}

void send_pulse(uint8_t length)
{
	ir_length = 12 * length + 1;	
}

void write_lcd(uint8_t character)
{
}

int main()
{
	int old_count = 0; // Timer counter previous value

	// Trigger debounce
	uint8_t trigger_count = 0;

	// Tone generation variables
	uint8_t total_note = 0;
	uint8_t total_note2 = 0;

	// Varaibles when sending IR
	uint8_t ir_data = 0;
	uint8_t header_sent = 0;
	uint8_t bit_sent = 0;

	// Variables when receiving IR
	uint8_t header_received = 0;
	uint8_t gap_begin = 0;
	uint8_t bit_begin = 0;
	uint8_t header_begin = 0;
	uint8_t total_bits = 0;

	enum state current_state = INIT; // start out in initialization

	setup();

	// Set LCD message
	write_byte('R',0);
	write_byte('e',0);
	write_byte('a',0);
	write_byte('d',0);
	write_byte('y',0);
	write_byte('!',0);
	
	// Turn on the power LED
	PORTB |= STATUS_PIN;

	while(1)
	{
		if(trigger_down)
		{
			PORTB &= !STATUS_PIN;
		} else {
			PORTB |= STATUS_PIN;
		}

		if(old_count != cycle_count)
		{
			if(trigger_down)
			{
				trigger_count++;
			} else {
				trigger_count = 0;
			}

			if(total_note)
			{
				total_note--;
			} else if (!total_note && total_note2) {
				total_note2--;
				total_note = 0xff;
			} 

			if(header_received < HEADER_LENGTH)
			{
				if(ir_detect)
				  header_received++;
			}


			if(header_received >= HEADER_LENGTH)
			{
				PORTD &= ~HIT_PIN;
			} else {
				PORTD |= HIT_PIN;
			}

			old_count = cycle_count;
		}

		if(trigger_count > 25)
		{
			trigger_count = 0;
			total_note = 0xff;
			total_note2 = 0x55;
			play_note(0x10,0xff);
			write_byte(0x02,1);
			write_bytes("Button pressed", 14);
			/*write_byte('B', 0);
			write_byte('u', 0);
			write_byte('t', 0);
			write_byte('t', 0);
			write_byte('o', 0);
			write_byte('n', 0);
			write_byte(' ', 0);
			write_byte('p', 0);
			write_byte('r', 0);
			write_byte('e', 0);
			write_byte('s', 0);
			write_byte('s', 0);
			write_byte('e', 0);
			write_byte('d', 0);*/
			send_pulse(HEADER_LENGTH); // send the header
			ir_high = 1;
			ir_data = IR_READY;
		}

		if(!ir_length && ir_data) // Data in the buffer and previous bit sent
		{
			if(header_sent) // Header has been sent, send a gap
			{
				header_sent = 0;
				ir_high = 0;
				send_pulse(1);
			} else if(bit_sent) { // A bit was sent, send a gap
				bit_sent = 0;
				ir_high = 0;
				send_pulse(1);
			} else if(ir_data & 80) // Send a high bit
			{
				bit_sent = 1;
				ir_high = 1;
				send_pulse(1); 
			} else { // Send a low bit
				bit_sent = 1;
				ir_high = 0;
				send_pulse(1);
			}
		}



		if(!total_note2)
		{
			play_note(0x00, 0x00);
		}
	}

	return 0;
}
