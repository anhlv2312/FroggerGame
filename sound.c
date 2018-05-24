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

#define F_CPU 8000000UL	// 8MHz

void init_sound(void) {
	DDRD = (1<<4);
}