/*
 * PinPortMapping.h
 *
 *  Created on: 01.12.2017
 *      Author: peterzdankin
 */

#ifndef PINPORTMAPPING_H_
#define PINPORTMAPPING_H_

#include <avr/pgmspace.h>

extern const uint16_t PROGMEM port_to_output_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[];
extern const uint8_t PROGMEM digital_pin_to_port_PGM[];

#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_output_PGM + (P))) )
#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )

#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7
#define PH 8
#define PJ 10
#define PK 11
#define PL 12

#define NOT_A_PORT 0


#endif /* PINPORTMAPPING_H_ */