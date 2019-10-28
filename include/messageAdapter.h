/*
 * messageAdapter.h
 *
 *  Created on: May 21, 2019
 *      Author: kenan
 */

#ifndef INCLUDE_MESSAGEADAPTER_H_
#define INCLUDE_MESSAGEADAPTER_H_

#include "neo_pixel.h"

void setMessageToLED(Adafruit_NeoPixel *p, const unsigned char *payload, int size);

#endif /* INCLUDE_MESSAGEADAPTER_H_ */
