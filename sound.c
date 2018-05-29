/*
 * sound.c
 *
 * Created: 24/05/2018 2:35:27 PM
 *  Author: Andy LE
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL	// 8MHz
#include <util/delay.h>
#include "sound.h"

void start_sound(void);
void stop_sound(void);
void play_sound(uint16_t, uint16_t);

uint32_t sound_count, sound_max_count, clockperiod, pulsewidth = 0;


void init_sound() {
	
	// Make pin OC1B be an output (port D, pin 4)
	DDRD = (1<<6);
	
	OCR2A = 255;
	OCR2B = 0;
	
	// Set up timer/counter 1 for Fast PWM, counting from 0 to the value in OCR1A
	// before reseting to 0. Count at 1MHz (CLK/8).
	// Configure output OC1B to be clear on compare match and set on timer/counter
	// overflow (non-inverting mode).
	TCCR2A = (1 << COM2B1) | (0 <<COM2B0) | (1 <<WGM21) | (1 << WGM20);
	TCCR2B = (1 << WGM22);

	TIMSK2 = (1<<OCIE2B);
	sei();
	
}

void play_new_game_sound() {
	for (uint8_t i=0; i<1; i++) {
		
	play_sound(4, 140);
	_delay_ms(100);
	stop_sound();
	_delay_ms(50);
	play_sound(4, 140);
	_delay_ms(100);
	stop_sound();
	_delay_ms(50);
	play_sound(3, 140);
	_delay_ms(100);
	stop_sound();
	_delay_ms(50);
	play_sound(2, 140);
	_delay_ms(200);
	stop_sound();
	_delay_ms(100);
	play_sound(4, 140);
	_delay_ms(50);
	stop_sound();
	_delay_ms(50);
	play_sound(3, 140);
	_delay_ms(50);
	stop_sound();
	_delay_ms(50);
	play_sound(2, 140);
	_delay_ms(200);
	}

}

void play_next_level_sound() {
	for (uint8_t i=0; i<1; i++) {
	
		play_sound(4, 140);
		_delay_ms(50);
		stop_sound();
		_delay_ms(50);
		play_sound(3, 140);
		_delay_ms(50);
		stop_sound();
		_delay_ms(50);
		play_sound(2, 140);
		_delay_ms(200);
	}

}

void play_game_over_sound() {
	for (uint8_t i=0; i<1; i++) {
		play_sound(20, 140);
		_delay_ms(200);
		stop_sound();
		_delay_ms(50);
		play_sound(30, 140);
		_delay_ms(200);
		stop_sound();
		_delay_ms(50);
		play_sound(40, 140);
		_delay_ms(200);
		stop_sound();
		_delay_ms(50);
		play_sound(60, 140);
		_delay_ms(400);
	}

}


void play_sound2(uint16_t freq, uint16_t length) {
	
	uint16_t clockperiod = 8000000L/(1024L*freq);
	
	OCR2A = clockperiod;
	if ((PINA & (1<<3)) >>3) {
		OCR2B = clockperiod/2;
		} else {
		OCR2B = 1;
	}
	

	sound_max_count = length*freq/1000L;
	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width - unless the pulse width is 0.

	start_sound();
}

void play_sound(uint16_t ocr, uint16_t count) {
	OCR2A = ocr;
	if ((PINA & (1<<3)) >>3) {
		OCR2B = ocr/2;
	} else {
		OCR2B = 1;
	}

	sound_max_count = count;
	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width - unless the pulse width is 0.

	start_sound();
}

void play_click_sound() {
	play_sound(20, 20);
	//play_sound2(20, 4000);
}

void play_frog_die_sound() {
	play_sound(60, 40);
}

void start_sound() {
	sound_count = 0;
	if ((PINA & (1<<4)) >>4) {
		TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
	}
}

void stop_sound() {
	TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
}


ISR(TIMER2_COMPB_vect) {
	/* Increment our clock tick count */
	sound_count++;
	if (sound_count > sound_max_count) {
		stop_sound();
	}
}

