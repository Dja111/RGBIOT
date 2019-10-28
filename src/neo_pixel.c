/*
 * neo_pixel.c
 *
 * Created on: 29.04.2019
 * Author: kenan-kajkus
 */

#include"../include/neo_pixel.h"
#include<stdlib.h>
#include<avr/io.h>
//#include"../configurations.h"
#include <string.h>
#include <stdlib.h>
#include<stdbool.h>
#include "../include/Timer.h"
#include<util/delay.h>
#include<stdio.h>

#include <avr/interrupt.h>
#include "../include/PinPortMapping.h"

#define PIXELDDRX      DDRF
#define PIXELPORT      PORTF

// Constructor when length, pin and type are known at compile-time:
Adafruit_NeoPixel *createNeoPixel(uint16_t n, uint8_t p, neoPixelType t){
	Adafruit_NeoPixel *adaNeo = malloc(sizeof(Adafruit_NeoPixel));
	adaNeo->begun = false;
	adaNeo->brightness = 0;
	adaNeo->pixels = NULL;
	adaNeo->endTime = 0;
  updateType(adaNeo, t);
  updateLength(adaNeo, n);
  setPin(adaNeo, p);
  return adaNeo;
}

Adafruit_NeoPixel *createNeoPixelNoParams(void) {
	Adafruit_NeoPixel *adaNeo = malloc(sizeof(Adafruit_NeoPixel));

	adaNeo->is800KHz = true;
	adaNeo->begun = false;
	adaNeo->numLEDs = 0;
	adaNeo->numBytes = 0;
	adaNeo->pin = -1;
	adaNeo->brightness = 0;
	adaNeo->pixels = NULL;
	adaNeo->rOffset = 1;
	adaNeo->gOffset = 0;
	adaNeo->bOffset = 2;
	adaNeo->wOffset = 1;
	adaNeo->endTime = 0;

	return adaNeo;
}

void freeNeoPixel(Adafruit_NeoPixel *adaNeo){
	if(adaNeo->pixels){
		free(adaNeo->pixels);
	}
	if(adaNeo->pin>=0){
		PIXELDDRX &= ~(1<<adaNeo->pin);
	}
	free(adaNeo);
}

void begin(Adafruit_NeoPixel *adaNeo) {
  if(adaNeo->pin >= 0) {
	  PIXELDDRX |= 1<<adaNeo->pin;
	  PIXELPORT &= ~(1<<adaNeo->pin);
  }
  sei();
  setup_timer();
  adaNeo->begun = true;
}

void updateLength(Adafruit_NeoPixel *adaNeo, uint16_t n) {
  if(adaNeo->pixels){
	  free(adaNeo->pixels); // Free existing data (if any)
  }
  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  adaNeo->numBytes = n * ((adaNeo->wOffset == adaNeo->rOffset) ? 3 : 4);//RGB or RGBW
  if((adaNeo->pixels = (uint8_t *)malloc(adaNeo->numBytes))) {
    memset(adaNeo->pixels, 0, adaNeo->numBytes);
    adaNeo->numLEDs = n;
  } else {
	  adaNeo->numLEDs = adaNeo->numBytes = 0;
  }
}

void updateType(Adafruit_NeoPixel *adaNeo, neoPixelType t) {
  bool oldThreeBytesPerPixel = (adaNeo->wOffset == adaNeo->rOffset); // false if RGBW

  adaNeo->wOffset = (t >> 6) & 0b11; // See notes in header file
  adaNeo->rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  adaNeo->gOffset = (t >> 2) & 0b11;
  adaNeo->bOffset =  t       & 0b11;
  adaNeo->is800KHz = (t < 256);      // 400 KHz flag is 1<<8

  // If bytes-per-pixel has changed (and pixel data was previously
  // allocated), re-allocate to new size.  Will clear any data.
  if(adaNeo->pixels) {
    bool newThreeBytesPerPixel = (adaNeo->wOffset == adaNeo->rOffset);
    if(newThreeBytesPerPixel != oldThreeBytesPerPixel){
    	updateLength(adaNeo,adaNeo->numLEDs);
    	}
  }
}

void show(Adafruit_NeoPixel *adaNeo) {


  if(!adaNeo->pixels) return;


  // Data latch = 300+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while(!canShow(adaNeo));
  // endTime is a private member (rather than global var) so that mutliple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions.  It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT.  The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.

// AVR MCUs -- ATmega & ATtiny (no XMEGA) ---------------------------------

  volatile uint16_t i = adaNeo->numBytes; // Loop counter
  volatile uint8_t
   *ptr = adaNeo->pixels,   // Pointer to next byte
    b   = *ptr++,   // Current byte value
    hi,             // PORT w/output bit set high
    lo;             // PORT w/output bit set low

  // Hand-tuned assembly code issues data to the LED drivers at a specific
  // rate.  There's separate code for different CPU speeds (8, 12, 16 MHz)
  // for both the WS2811 (400 KHz) and WS2812 (800 KHz) drivers.  The
  // datastream timing for the LED drivers allows a little wiggle room each
  // way (listed in the datasheets), so the conditions for compiling each
  // case are set up for a range of frequencies rather than just the exact
  // 8, 12 or 16 MHz values, permitting use with some close-but-not-spot-on
  // devices (e.g. 16.5 MHz DigiSpark).  The ranges were arrived at based
  // on the datasheet figures and have not been extensively tested outside
  // the canonical 8/12/16 MHz speeds; there's no guarantee these will work
  // close to the extremes (or possibly they could be pushed further).
  // Keep in mind only one CPU speed case actually gets compiled; the
  // resulting program isn't as massive as it might look from source here.
// 16 MHz(ish) AVR --------------------------------------------------------
    // WS2811 and WS2812 have different hi/lo duty cycles; this is
    // similar but NOT an exact copy of the prior 400-on-8 code.

    // 20 inst. clocks per bit: HHHHHxxxxxxxxLLLLLLL
    // ST instructions:         ^   ^        ^       (T=0,5,13)

    volatile uint8_t n1, n2 =0;

    hi   = *adaNeo->port |  adaNeo->pinMask;
    lo   = *adaNeo->port & ~(adaNeo->pinMask);
    n1 = lo;
    if(b & 0x80) n1 = hi;

    

    asm volatile(
       "headF:"                   "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 6"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 5"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 4"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 3"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 2"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 1"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 0"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "sbiw %[count], 1"        "\n\t"
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "ld   %[byte] , %a[ptr]+" "\n\t"
        "sbrc %[byte] , 7"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "brne headF"              "\n"
      : [byte] "+r" (b), [n1] "+r" (n1), [n2] "+r" (n2), [count] "+w" (i)
      : [port] "I" (_SFR_IO_ADDR(PORTF)), [ptr] "e" (ptr), [hi] "r" (hi),
        [lo] "r" (lo));
    reset_timer();
    //adaNeo->endTime = micros(); // Save EOD time for latch on next call
}

// Set the output pin number
void setPin(Adafruit_NeoPixel *adaNeo, uint8_t p) {
  if(adaNeo->begun && (adaNeo->pin >= 0)){
	  PIXELDDRX &=~(1<<adaNeo->pin);
  }
  	adaNeo->pin = p;
    if(adaNeo->begun) {
    	PIXELDDRX |= 1<<p;
    	PIXELPORT &= ~(1<<p);
    }
    adaNeo->port    = portOutputRegister(digitalPinToPort(p));
    	adaNeo->pinMask = digitalPinToBitMask(p);
}

// Set pixel color from separate R,G,B components:
void setPixelColorRGB(Adafruit_NeoPixel *adaNeo, uint16_t n, uint8_t r, uint8_t g, uint8_t b) {

  if(n < adaNeo->numLEDs) {
    if(adaNeo->brightness) { // See notes in setBrightness()
      r = (r * adaNeo->brightness) >> 8;
      g = (g * adaNeo->brightness) >> 8;
      b = (b * adaNeo->brightness) >> 8;
    }
    uint8_t *p;
    if(adaNeo->wOffset == adaNeo->rOffset) { // Is an RGB-type strip
      p = &adaNeo->pixels[n * 3];    // 3 bytes per pixel
    } else {                 // Is a WRGB-type strip
      p = &adaNeo->pixels[n * 4];    // 4 bytes per pixel
      p[adaNeo->wOffset] = 0;        // But only R,G,B passed -- set W to 0
    }
    p[adaNeo->rOffset] = r;          // R,G,B always stored
    p[adaNeo->gOffset] = g;
    p[adaNeo->bOffset] = b;
  }
}

void setPixelColorRGBW(Adafruit_NeoPixel *adaNeo, uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

  if(n < adaNeo->numLEDs) {
    if(adaNeo->brightness) { // See notes in setBrightness()
      r = (r * adaNeo->brightness) >> 8;
      g = (g * adaNeo->brightness) >> 8;
      b = (b * adaNeo->brightness) >> 8;
      w = (w * adaNeo->brightness) >> 8;
    }
    uint8_t *p;
    if(adaNeo->wOffset == adaNeo->rOffset) { // Is an RGB-type strip
      p = &adaNeo->pixels[n * 3];    // 3 bytes per pixel (ignore W)
    } else {                 // Is a WRGB-type strip
      p = &adaNeo->pixels[n * 4];    // 4 bytes per pixel
      p[adaNeo->wOffset] = w;        // Store W
    }
    p[adaNeo->rOffset] = r;          // Store R,G,B
    p[adaNeo->gOffset] = g;
    p[adaNeo->bOffset] = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void setPixelColor32Bit(Adafruit_NeoPixel *adaNeo, uint16_t n, uint32_t c) {

  if(n < adaNeo->numLEDs) {
    uint8_t *p,
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if(adaNeo->brightness) { // See notes in setBrightness()
      r = (r * adaNeo->brightness) >> 8;
      g = (g * adaNeo->brightness) >> 8;
      b = (b * adaNeo->brightness) >> 8;
    }
    if(adaNeo->wOffset == adaNeo->rOffset) {
      p = &adaNeo->pixels[n * 3];
    } else {
      p = &adaNeo->pixels[n * 4];
      uint8_t w = (uint8_t)(c >> 24);
      p[adaNeo->wOffset] = adaNeo->brightness ? ((w * adaNeo->brightness) >> 8) : w;
    }
    p[adaNeo->rOffset] = r;
    p[adaNeo->gOffset] = g;
    p[adaNeo->bOffset] = b;
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t ColorRGB(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Convert separate R,G,B,W into packed 32-bit WRGB color.
// Packed format is always WRGB, regardless of LED strand color order.
uint32_t ColorRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t getPixelColor(Adafruit_NeoPixel *adaNeo, uint16_t n){
  if(n >= adaNeo->numLEDs) return 0; // Out of bounds, return no color.

  uint8_t *p;

  if(adaNeo->wOffset == adaNeo->rOffset) { // Is RGB-type device
    p = &adaNeo->pixels[n * 3];
    if(adaNeo->brightness) {
      // Stored color was decimated by setBrightness().  Returned value
      // attempts to scale back to an approximation of the original 24-bit
      // value used when setting the pixel color, but there will always be
      // some error -- those bits are simply gone.  Issue is most
      // pronounced at low brightness levels.
      return (((uint32_t)(p[adaNeo->rOffset] << 8) / adaNeo->brightness) << 16) |
             (((uint32_t)(p[adaNeo->gOffset] << 8) / adaNeo->brightness) <<  8) |
             ( (uint32_t)(p[adaNeo->bOffset] << 8) / adaNeo->brightness       );
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[adaNeo->rOffset] << 16) |
             ((uint32_t)p[adaNeo->gOffset] <<  8) |
              (uint32_t)p[adaNeo->bOffset];
    }
  } else {                 // Is RGBW-type device
    p = &adaNeo->pixels[n * 4];
    if(adaNeo->brightness) { // Return scaled color
      return (((uint32_t)(p[adaNeo->wOffset] << 8) / adaNeo->brightness) << 24) |
             (((uint32_t)(p[adaNeo->rOffset] << 8) / adaNeo->brightness) << 16) |
             (((uint32_t)(p[adaNeo->gOffset] << 8) / adaNeo->brightness) <<  8) |
             ( (uint32_t)(p[adaNeo->bOffset] << 8) / adaNeo->brightness       );
    } else { // Return raw color
      return ((uint32_t)p[adaNeo->wOffset] << 24) |
             ((uint32_t)p[adaNeo->rOffset] << 16) |
             ((uint32_t)p[adaNeo->gOffset] <<  8) |
              (uint32_t)p[adaNeo->bOffset];
    }
  }
}

// Returns pointer to pixels[] array.  Pixel data is stored in device-
// native format and is not translated here.  Application will need to be
// aware of specific pixel data format and handle colors appropriately.
uint8_t *getPixels(Adafruit_NeoPixel *adaNeo){
  return adaNeo->pixels;
}

uint16_t numPixels(Adafruit_NeoPixel *adaNeo){
  return adaNeo->numLEDs;
}

// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void setBrightness(Adafruit_NeoPixel *adaNeo,uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if(newBrightness != adaNeo->brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = adaNeo->pixels,
             oldBrightness = adaNeo->brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    uint16_t i;
    for(i=0; i<adaNeo->numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    adaNeo->brightness = newBrightness;
  }
}

uint8_t getBrightness(Adafruit_NeoPixel *adaNeo){
  return adaNeo->brightness - 1;
}

void clear(Adafruit_NeoPixel *adaNeo) {
  memset(adaNeo->pixels, 0, adaNeo->numBytes);
}

int8_t getPin(Adafruit_NeoPixel *adaNeo) { return adaNeo->pin; };

//300 Microseconds time needed to update the lights
bool canShow(Adafruit_NeoPixel *adaNeo) {
return micros() >= 300L; }


