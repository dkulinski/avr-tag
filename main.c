#define F_CPU 16000000

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define STATUS_PIN 32 // Status LED is on PB5
#define MOSFET_PIN 2  // MOSFET is driven on PB1

#define HIT_PIN 128   // Hit LEDs are driven from PD7
#define BUZZER_PIN 8  // Piezo buzzer driven on PD3

int main()
{
	return 0;
}
