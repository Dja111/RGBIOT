/*
 * messageAdapter.c
 *
 *  Created on: May 21, 2019
 *      Author: kenan
 */

#include "../include/neo_pixel.h"

void setMessageToLED(Adafruit_NeoPixel *p, const unsigned char *payload, int size)
{
	int i = 0;
	for(i=0;i<size;i=i+4)
	{
		if(payload[i]!=0)
		{
			setPixelColorRGB(p,payload[i]-1,payload[i+1],payload[i+2],payload[i+3]);
		}

	}
}
