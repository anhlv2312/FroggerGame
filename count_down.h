/*
 * timer1.h
 *
 * Created: 23/05/2018 8:38:57 AM
 *  Author: Andy LE
 */ 

#ifndef COUNT_DOWN_H_
#define COUNT_DOWN_H_

#include <stdint.h>

void init_count_down();
void start_count_down(uint16_t);
void pause_count_down(uint8_t);
void stop_count_down();
uint8_t is_timed_out();

#endif