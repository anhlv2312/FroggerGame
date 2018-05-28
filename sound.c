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

// For a given frequency (Hz), return the clock period (in terms of the
// number of clock cycles of a 1MHz clock)
uint16_t freq_to_clock_period(uint16_t freq) {
	return (1000000UL / freq);	// UL makes the constant an unsigned long (32 bits)
	// and ensures we do 32 bit arithmetic, not 16
}

// Return the width of a pulse (in clock cycles) given a duty cycle (%) and
// the period of the clock (measured in clock cycles)
uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod) {
	return (dutycycle * clockperiod) / 100;
}

int init_sound() {
	uint16_t freq = 200;	// Hz
	float dutycycle = 2;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Make pin OC1B be an output (port D, pin 4)
	DDRD = (1<<4);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;
	
	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width - unless the pulse width is 0.
	if(pulsewidth == 0) {
		OCR1B = 0;
		} else {
		OCR1B = pulsewidth - 1;
	}
	
	// Set up timer/counter 1 for Fast PWM, counting from 0 to the value in OCR1A
	// before reseting to 0. Count at 1MHz (CLK/8).
	// Configure output OC1B to be clear on compare match and set on timer/counter
	// overflow (non-inverting mode).
	TCCR1A = (1 << COM1B1) | (0 <<COM1B0) | (1 <<WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);

	// PWM output should now be happening - at the frequency and pulse width set above
	
	while(1) {
		// Check the state of the buttons (on port C) every 100ms.
		_delay_ms(100);
		
		if(PINC & 0x01) { // increase frequency by 5%, but highest frequency is 10000Hz
			freq = freq*105UL/100UL;	// Constants made 32 bit to ensure 32 bit arithmetic
			if(freq > 10000) {
				freq = 10000;
			}
		}
		if(PINC & 0x02) { // decrease frequency by 5%, but lowest frequency is 20Hz
			freq = freq*95UL/100UL;		// Constants made 32 bits to ensure 32 bit arithmetic
			if(freq < 20) {
				freq = 20;
			}
		}
		if(PINC & 0x04) { // increase duty cycle by 0.1 if less than 10% or 1 if 10% or higher
			if(dutycycle < 10) {
				dutycycle += 0.1;
				} else {
				dutycycle += 1.0;
				if(dutycycle > 100) {
					dutycycle = 100;
				}
			}
		}
		if(PINC & 0x08) { // decrease duty cycle by 0.1 if less than 10% or 1 if 10% or higher
			if(dutycycle < 10) {
				dutycycle -= 0.1;
				if(dutycycle < 0) {
					dutycycle = 0;
				}
				} else {
				dutycycle -= 1.0;
			}
		}
		
		// Work out the clock period and pulse width
		clockperiod = freq_to_clock_period(freq);
		pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
		
		// Update the PWM registers
		if(pulsewidth > 0) {
			// The compare value is one less than the number of clock cycles in the pulse width
			OCR1B = pulsewidth - 1;
			} else {
			OCR1B = 0;
		}
		// Note that a compare value of 0 results in special behaviour - see page 169 of the
		// datasheet (10/2016 version)
		
		// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
		OCR1A = clockperiod - 1;
	}
}

