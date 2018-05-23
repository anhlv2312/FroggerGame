/*
 * joystick.c
 *
 * Created: 23/05/2018 12:12:51 PM
 *  Author: Andy LE
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "joystick.h"

uint16_t value;
uint8_t x_or_y = 0;	/* 0 = x, 1 = y */

static volatile int8_t joystick;

void init_joystick(void) {
	/* Turn on global interrupts */
	sei();
		
	// Set up ADC - AVCC reference, right adjust
	// Input selection doesn't matter yet - we'll swap this around in the while
	// loop below.
	
	ADMUX = (1<<REFS0) | (1<<MUX1) | (1<<MUX2);
	
	// Turn on the ADC (but don't start a conversion yet). Choose a clock
	// divider of 64. (The ADC clock must be somewhere
	// between 50kHz and 200kHz. We will divide our 8MHz clock by 64
	// to give us 125kHz.)
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADIE);
	
	ADCSRA |= (1<<ADSC);
}

uint8_t joystick_moved(void) {
	return joystick; 
}

ISR(ADC_vect) {		
	value = ADC; // read the value
	if(x_or_y == 0) {
		ADMUX &= ~1;
		} else {
		ADMUX |= 1;
	}
	
	if (value > (512 + 256)) {
		if(x_or_y == 0) {
			joystick = 2;
			} else {
			joystick = 0;
			}
	} else if (value < (512 - 256)) {
		if(x_or_y == 0) {
			joystick = 1;
		} else {
			joystick= 3;
		}
	} else {
		joystick = NO_JOYSTICK_MOVED;
	}
	x_or_y ^= 1;

	ADCSRA |= (1<<ADSC);
}