/*
 * sound.c
 *
 * Created: 24/05/2018 2:35:27 PM
 *  Author: Andy LE
 */ 

#include <avr/io.h>
#define F_CPU 8000000UL	// 8MHz
#include <util/delay.h>
#include <stdint.h>

void init_sound(void) {
	DDRA = (1<<5);
	uint16_t freq = 200;	// Hz
	float dutycycle = 2;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;
		
	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width - unless the pulse width is 0.
	if(pulsewidth == 0) {
		OCR1B = 0;
		} else {
		OCR1B = pulsewidth - 1;
	}
	
	TCCR1A = (1 << COM1B1) | (0 <<COM1B0) | (1 <<WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);
	
}

void play_sound(void) {
	
	
}