/*
 * joystick.h
 *
 * Created: 23/05/2018 12:13:18 PM
 *  Author: Andy LE
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#define NO_JOYSTICK_MOVED (-1)

void init_joystick (void);
uint8_t joystick_moved(void);


#endif /* JOYSTICK_H_ */