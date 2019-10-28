/*
 * millis.c
 *
 *  Created on: Jun 5, 2019
 *      Author: kenan
 */
#include "../include/millis.h"
#include <avr/io.h>
#include <util/atomic.h>

#define CTC_MATCH_OVERFLOW ((F_CPU / 1000)/8)

ISR(TIMER3_COMPA_vect)
{
	timer2_millis++;
}

void init_millis()
{
	//CTC mode , prescaler 8
	TCCR3B |= (1<<WGM32)|(1<<CS31);
	OCR3AH = (CTC_MATCH_OVERFLOW >> 8);
	OCR3AL = CTC_MATCH_OVERFLOW;
	TIMSK3 |= (1<< OCIE3A);

}

unsigned long millis()
{
	unsigned long millis_return;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		millis_return = timer2_millis;
	}
	return millis_return;
}

