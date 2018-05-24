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
	ADMUX = (1<<REFS0) | (1<<MUX1) | (1<<MUX2);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); // | (1<<ADIE);
	ADCSRA |= (1<<ADSC);
}

int8_t get_x(void) {
	int8_t x = 0;
	ADMUX |= 1;
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	if (ADC > (512 + 256)) {
		x = 1;
	} else if (ADC < (512 - 256)) {
		x = -1;
	}
	return x;
}

int8_t get_y(void) {
	int8_t y = 0;
	ADMUX &= ~1;
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	if (ADC > (512 + 256)) {
		y = 1;
	} else if (ADC < (512 - 256)) {
		y = -1;
	}
	return y;
}