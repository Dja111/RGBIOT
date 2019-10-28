/*
 * Timer.h
 *
 *  Created on: 01.12.2017
 *      Author: peterzdankin
 */

#ifndef TIMER_H_
#define TIMER_H_

#define PRESCALER 8

void setup_timer(void);
uint32_t micros(void);
void reset_timer(void);
void reset_and_start_timer(void);

#endif /* TIMER_H_ */
