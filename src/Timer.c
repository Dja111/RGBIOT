
#include <avr/interrupt.h>
#include "include/Timer.h"
#include <util/delay.h>

volatile unsigned long timer1_millis;

uint32_t elapsed_time = 0;
volatile uint32_t overflow_count = 65530;


void reset_and_start_timer()
{
	TIMSK1 |= (1 << TOIE1);	// Enable overflow interrupt
	TCNT1 = 0x0000;	// Reset counter
}

void reset_timer(){
	overflow_count = 0;
	elapsed_time = 0;
	TCNT1 = 0;
}

void setup_timer()
{
	TCCR1A = 0x0000;	// Enable Timer1 Normal mode
	TCCR1B |= (1 << CS11);	// Set prescaler to 8
	reset_and_start_timer();
}
uint32_t micros() {
	elapsed_time = (TCNT1 + overflow_count*65536) / ((F_CPU/1000000)/PRESCALER);
	return elapsed_time;
}

ISR(TIMER1_OVF_vect)
{
	++overflow_count;
}

void delay_microseconds(uint16_t microseconds) {
	while (microseconds > 0){
		_delay_us(1);
		microseconds--;
	}
}
