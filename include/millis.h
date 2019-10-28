/*
 * millis_timer3.h
 *
 *  Created on: Jun 5, 2019
 *      Author: kenan
 */

#ifndef INCLUDE_MILLIS_H_
#define INCLUDE_MILLIS_H_
volatile unsigned long timer2_millis;

void init_millis(void);
unsigned long millis(void);

#endif /* INCLUDE_MILLIS_H_ */
