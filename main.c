#define F_CPU 16000000

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define STATUS_PIN 32 // Status LED is on PB5
#define MOSFET_PIN 2  // MOSFET is driven on PB1

#define HIT_PIN 128   // Hit LEDs are driven from PD7
#define BUZZER_PIN 8  // Piezo buzzer driven on PD3

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

int main()
{
	return 0;
}
