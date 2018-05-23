

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer1.h"

volatile uint8_t count_down_timing = 0;
volatile uint8_t timed_out = 1;
volatile uint16_t count = 0;

#define SEVEN_SEG_CC ((PIND & (1<<7)) >> 7)

uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};

void init_timer1() {
	DDRC = 0xFF;
	DDRD |= (1<<7);

	/* Set up timer/counter 1 so that we get an 
	** interrupt 100 times per second, i.e. every
	** 10 milliseconds.
	*/
	OCR1A = 9999; /* Clock divided by 8 - count for 10000 cycles */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11); /* Divide clock by 8 */

	/* Enable interrupt on timer on output compare match 
	*/
	TIMSK1 = (1<<OCIE1A);

	/* Ensure interrupt flag is cleared */
	TIFR1 = (1<<OCF1A);

	/* Turn on global interrupts */
	sei();
	
}

void start_count_down(uint16_t second) {
	count = (second + 1) * 100;
	count_down_timing = 1;
	timed_out = 0;
}

void pause_count_down(uint8_t paused) {
	count_down_timing = 1 ^ paused;
}

void stop_count_down() {
	count = 0;
	count_down_timing = 0;
	timed_out = 1;
}

uint8_t is_timed_out() {
	return timed_out;
}

ISR(TIMER1_COMPA_vect) {

	if(count_down_timing) {
		count--;
		if(count == 0) {
			count_down_timing = 0;
			timed_out = 1;
		}
	}

	// Flip the PortD's 7th Pin
	PORTD ^= (1<<7);
	
	if(!timed_out) {
		if(SEVEN_SEG_CC == 0) {
			if (count < 100) {
				PORTC = seven_seg_data[(count/10)%10];
			} else {
				PORTC = seven_seg_data[(count/100)%10];
			}
		} else { 
			if (count < 100) {
				PORTC = seven_seg_data[0] | 0x80;
			} else if (count < 1000) {
				PORTC = 0;
			} else {
				PORTC = seven_seg_data[(count/1000)%10];
			}
		}	
	} else {
		if(SEVEN_SEG_CC == 0) {
			PORTC = seven_seg_data[0];
		} else {
			PORTC = seven_seg_data[0] | 0x80;
		}
	}

}
