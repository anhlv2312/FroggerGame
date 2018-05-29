/*
 * FroggerProject.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by <YOUR NAME HERE>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "count_down.h"
#include "sound.h"
#include "joystick.h"
#include "game.h"

#define F_CPU 8000000L
#include <util/delay.h>

uint8_t live_led_data[4] = {0b0, 0b1, 0b11, 0b111};
	
uint8_t frog_live;
uint8_t game_level;

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void update_score(void);
void update_live(void);
void update_level(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27
#define MAX_LIVE 3
#define COUNT_DOWN 20 //sec
#define SPEED_STEP 100
#define BUTTON_UP ((PINB & (1<<2)) >> 2)
#define BUTTON_DOWN ((PINB & (1<<1)) >> 1)
#define BUTTON_LEFT ((PINB & (1<<3)) >> 3) 
#define BUTTON_RIGHT ((PINB & (1<<0)) >> 0) 


/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);

	init_timer0();
	init_count_down();
	init_sound();
	
	init_joystick();
	
	// Set 3 pins of port D to be the out put for lives
	DDRA |= 0b111;
	
	// Set 8 pin of port C to be the out put for 7 segs display
	DDRC = 0xFF;
	
	// Set pin 5 of port D to be 7-Segment CC
	DDRD |= (1<<DDRD5);
	
	// Turn on global interrupts
	sei();
}

void splash_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_cursor(10,10);
	printf_P(PSTR("Frogger"));
	move_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 project by Vu Anh LE"));
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	while(1) {
		set_scrolling_display_text("FROGGER 44907635", COLOUR_GREEN);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed
		while(scroll_display()) {
			_delay_ms(150);
			if(button_pushed() != NO_BUTTON_PUSHED) {
				return;
			}
		}
	}
}

void new_game(void) {
	// Initialise the game and display
	initialise_game();
	
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the score
	init_score();
	update_score();
	
	frog_live = MAX_LIVE;
	update_live();
	game_level = 0;
	update_level();
	
	play_new_game_sound();

	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	uint32_t current_time;
	uint32_t button_press_time, joystick_press_time, hold_time;
	uint8_t holding_button, holding_joystick, holding_x, holding_y;
	int8_t button;
	int8_t paused;
	int8_t joystick_x, joystick_y;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
		
	// Get the current time and remember this as the last time the vehicles
	// and logs were moved.
	//current_time = get_current_time();
	//last_move_time = current_time;
	
	paused = 0;
	button_press_time = 0;
	joystick_press_time =0;
	hold_time = 0;
	holding_button = 0;
	holding_x =0;
	holding_y =0;
	
	// We play the game while the frog is alive and we haven't filled up the 
	// far riverbank
	start_count_down(COUNT_DOWN);
	
	while(!is_frog_dead()) {
		
		if(frog_has_reached_riverbank()) {
			// Frog reached the other side successfully but the
			// riverbank isn't full, put a new frog at the start;
			if (is_riverbank_full()) {
				play_next_level_sound();
				_delay_ms(500);
				for (uint8_t i=0; i<16; i++) {
					ledmatrix_shift_display_left();
					_delay_ms(75);
				}
				_delay_ms(500);
				clear_serial_input_buffer();
				if (frog_live < MAX_LIVE) {
					frog_live++;
					update_live();
				}
				game_level++;
				next_level(game_level);
				update_level();
			}
			put_frog_in_start_position();
			start_count_down(COUNT_DOWN);
		}
		
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. At most one of the following three
		// variables will be set to a value other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		joystick_x =0;
		joystick_y = 0;

		
		if(button == NO_BUTTON_PUSHED) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
					
		// Process the input. 
		if( button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
			// Attempt to move left
			if (!paused) {
				move_frog_to_left(); 
				play_click_sound();
			}
		} else if(button==2 || escape_sequence_char=='A' || serial_input=='U' || serial_input=='u') {
			// Attempt to move forward
			if (!paused) {
				move_frog_forward();
				play_click_sound();
				update_score();
			}
		} else if(button==1 || escape_sequence_char=='B' || serial_input=='D' || serial_input=='d') {
			// Attempt to move down
			if (!paused) {
				move_frog_backward();
				play_click_sound();
			}
		} else if(button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
			// Attempt to move right
			if (!paused) {
				move_frog_to_right();
				play_click_sound();
			}
		} else if(serial_input == 'p' || serial_input == 'P') {
			paused = 1 ^ paused;
			pause_count_down(paused);
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again
		}
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		
		current_time = get_current_time();
		
		if ((BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT) && !paused) {
			PCICR &= ~(1<<PCIE1);
			if (button_press_time == 0) {button_press_time = get_current_time();}
			hold_time = current_time - button_press_time;
			if (hold_time > 500 && hold_time % 200 ==0 ) {
				if (BUTTON_LEFT && (holding_button == 0 || holding_button == 1)) {
					move_frog_to_left();
					play_click_sound();
					holding_button = 1;
				} else if (BUTTON_RIGHT && (holding_button == 0 || holding_button == 2)) {
					move_frog_to_right();
					play_click_sound();
					holding_button = 2;
				} else if (BUTTON_UP && (holding_button == 0 || holding_button == 3)) {
					move_frog_forward();
					play_click_sound();
					holding_button = 3;
				} else if (BUTTON_DOWN && (holding_button == 0 || holding_button == 4)) {
					move_frog_backward();
					play_click_sound();
					holding_button = 4;
				}
			}
		} else {
			button_press_time = 0;
			holding_button = 0;
			PCICR |= (1<<PCIE1);
		}
		joystick_x = get_x();
		joystick_y = get_y();
		
		
		if ((joystick_x | joystick_y) && !paused) {
			if (joystick_press_time== 0) {joystick_press_time = get_current_time();}
			hold_time = current_time - joystick_press_time;
			if ((holding_joystick == 0 && hold_time>50) || (hold_time > 500 && hold_time %200 ==0)){
				if (joystick_y == 1 && joystick_x == 1) {
					move_frog_forward_right();
					holding_joystick = 1;
				} else if (joystick_y == 1 && joystick_x == -1) {
				move_frog_forward_left();
				holding_joystick = 1;
				} else if (joystick_y == -1 && joystick_x == 1) {
				move_frog_backward_right();
				holding_joystick = 1;
				} else if (joystick_y == -1 && joystick_x == -1) {
				move_frog_backward_left();
				holding_joystick = 1;
				} else if (joystick_x == 1) {
					move_frog_to_right();
					holding_joystick = 1;
				} else if (joystick_x == -1) {
				move_frog_to_left();
				holding_joystick = 1;
				} else if (joystick_y == 1) {
				move_frog_forward();
				holding_joystick = 1;
				} else if (joystick_y == -1) {
				move_frog_backward();
				holding_joystick = 1;			}
			}
		} else {
			joystick_press_time = 0;
			holding_joystick = 0;
		}

		
		if(!is_frog_dead() && !paused) {
			// 1000ms (1 second) has passed since the last time we moved
			// the vehicles and logs - move them again and keep track of
			// the time when we did this.
			if (current_time % (1100 - game_level * SPEED_STEP) == 0) {
				if (game_level % 2) {
					scroll_vehicle_lane(0, 1);
				} else {
					scroll_vehicle_lane(0, -1);
				}
			}
			if (current_time % (1000 - game_level * SPEED_STEP) == 0) {
				if (game_level % 2) {
					scroll_vehicle_lane(1, -1);
					} else {
					scroll_vehicle_lane(1, 1);
				}
			}
			if (current_time % (900 - game_level * SPEED_STEP) == 0) {
				if (game_level % 2) {
					scroll_vehicle_lane(2, 1);
					} else {
					scroll_vehicle_lane(2, -1);
				}
			}
			if (current_time % (1300 - game_level * SPEED_STEP) == 0) {
				if (game_level % 2) {
					scroll_river_channel(0, 1);
					} else {
					scroll_river_channel(0, -1);
				}
			}
			if (current_time % (1200 - game_level * SPEED_STEP) == 0) {
				if (game_level % 2) {
					scroll_river_channel(1, -1);
					} else {
					scroll_river_channel(1, 1);
				}
			}
		}

		if (is_timed_out()){
			set_frog_dead();
		}
	
		if (is_frog_dead()){
			play_frog_die_sound();
			frog_live--;
			update_live();
			pause_count_down(1);
			PCICR &= ~(1<<PCIE1);
			_delay_ms(2000);
			clear_serial_input_buffer();
			PCICR |= (1<<PCIE1);
			if (frog_live){
				put_frog_in_start_position();
				start_count_down(COUNT_DOWN);
			} 
		}
	}
	
	// We get here if the frog is dead or the riverbank is full
	// The game is over.
}

void handle_game_over() {
	stop_count_down();
	move_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	play_game_over_sound();
	_delay_ms(500);
	while(1) {
		set_scrolling_display_text("GAME OVER", COLOUR_ORANGE);
		// Scroll the message until it has scrolled off the
		// display or a button is pushed
		while(scroll_display()) {
			_delay_ms(150);
			if(button_pushed() != NO_BUTTON_PUSHED) {
				return;
			}
		}
	}
}

void update_score() {
	move_cursor(10,12);
	printf_P(PSTR("Score: %4d"), get_score());
}

void update_live() {
	move_cursor(30, 12);
	printf_P(PSTR("Live: %2d"), frog_live);
	PORTA &= ~(0b111);
	PORTA |= (live_led_data[frog_live]);
}

void update_level() {
	move_cursor(50, 12);
	printf_P(PSTR("Level: %2d"), game_level + 1);
}