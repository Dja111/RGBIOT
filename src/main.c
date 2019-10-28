#include <avr/io.h>
#include <util/delay.h>
#include "../include/neo_pixel.h"
#include "../include/millis.h"
#include "Setup/HardwareSetup.h"
#include "CommunicationModule/Mac802154MRFImpl.h"
#include "../include/messageAdapter.h"
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#define NUMPIXELS 	400
#define PIN			5

const int delayval = 1;
unsigned long milliseconds_since;

void showPixels_ms(unsigned long ms, Adafruit_NeoPixel *pixels)
{
	unsigned long milliseconds_current = millis();
	if(milliseconds_current > ms)
	{
		TCCR3B = 0;
		show(pixels);
		milliseconds_since = milliseconds_current;
	}
}

int main(void)
{
	//stuff copied from communicationModule Template, probably needs configuration
	setUpMac();
	Mac802154Config config = {
            .channel = 12,
            .pan_id = {0x19, 0x20},
            .short_source_address = {0xFF, 0xFF},
            .extended_source_address = {
                    0x00, 0x00,
                    0x00, 0x00,
                    0x00, 0x00,
					0xff, 0xff,
            },
	};
	Mac802154_configure(mac802154, &config);

	Adafruit_NeoPixel *pixels = createNeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
	begin(pixels);
	int i = 0;
	//running light to visualize boot up
	for(i=0;i<NUMPIXELS;i++)
	{
		setPixelColorRGB(pixels,i,0,0,0);
		setPixelColorRGB(pixels,i+1,255,0,0);
		show(pixels);
	}
		init_millis();
	const uint8_t *payload;
    while (1)
   	{
    	if(!Mac802154_newPacketAvailable(mac802154)){
    			showPixels_ms(200, pixels);
    	}
    	else{
    	wdt_enable(WDTO_120MS);
    	uint8_t size = Mac802154_getReceivedPacketSize(mac802154);
		uint8_t packet[size];
    	Mac802154_fetchPacketBlocking(mac802154, packet, size);
    	uint8_t payloadSize = Mac802154_getPacketPayloadSize(mac802154,packet);
    	wdt_reset();
    	payload = Mac802154_getPacketPayload(mac802154, packet);
      	wdt_reset();
      	wdt_disable();
      	setMessageToLED(pixels,payload,payloadSize);

    	TCCR3B |= (1<<WGM32)|(1<<CS31);
    	timer2_millis = 0;
    	}
   	}
/*
#include "integration_tests/src/Setup/HardwareSetup.h"
#include "CommunicationModule/Mac802154MRFImpl.h"
#include <stdio.h>
#include <string.h>

#include <util/atomic.h>

#define NUMPIXELS 	241
#define PIN			5


const int delayval = 2;

int main(void)
{
	setUpMac();
	Mac802154Config config = {
            .channel = 12,
            .pan_id = {0x0, 0xFF},
            .short_source_address = {0xAA, 0xAA},
            .extended_source_address = {
                    0x11, 0x22,
                    0x33, 0x44,
                    0x55, 0x66,
                    0x77, 0x88,
            },
    };

	Mac802154_configure(mac802154, &config);

	Adafruit_NeoPixel *pixels = createNeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
	int i = 0;
	begin(pixels);
    	while (1) 
    	{
		while(!Mac802154_newPacketAvailable(mac802154))
		{}
		uint8_t size = Mac802154_getReceivedPacketSize(mac802154);
		uint8_t packet[size];
		Mac802154_fetchPacketBlocking(mac802154, packet, size);
    		for(i = 0;i<NUMPIXELS;i++)
		{
    			setPixelColorRGB(pixels,i,254,0,0);
   			show(pixels);
			_delay_ms(delayval);
		}
  		for(i = 0;i<NUMPIXELS;i++)
		{
    			setPixelColorRGB(pixels,i,0,254,0);
    			show(pixels);
    			_delay_ms(delayval);
  		}
  		for(i = 0;i<NUMPIXELS;i++)
		{
	    		setPixelColorRGB(pixels,i,0,0,254);
	    		show(pixels);
	    		_delay_ms(delayval);
  		}

	}
>>>>>>> refs/heads/rebase*/
}
