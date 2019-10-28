/*
*NeoPixel.h
*Created on: 29.04.2019
*Author:kenan-kajkus
*
*This file is a rewrite of the Adafruit_NeoPixel library in order to compile in pure C
*
*/

#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include<stdint.h>
#include<stdbool.h>

// The order of primary colors in the NeoPixel data stream can vary
// among device types, manufacturers and even different revisions of
// the same item.  The third parameter to the Adafruit_NeoPixel
// constructor encodes the per-pixel byte offsets of the red, green
// and blue primaries (plus white, if present) in the data stream --
// the following #defines provide an easier-to-use named version for
// each permutation.  e.g. NEO_GRB indicates a NeoPixel-compatible
// device expecting three bytes per pixel, with the first byte
// containing the green value, second containing red and third
// containing blue.  The in-memory representation of a chain of
// NeoPixels is the same as the data-stream order; no re-ordering of
// bytes is required when issuing data to the chain.

// Bits 5,4 of this value are the offset (0-3) from the first byte of
// a pixel to the location of the red color byte.  Bits 3,2 are the
// green offset and 1,0 are the blue offset.  If it is an RGBW-type
// device (supporting a white primary in addition to R,G,B), bits 7,6
// are the offset to the white byte...otherwise, bits 7,6 are set to
// the same value as 5,4 (red) to indicate an RGB (not RGBW) device.
// i.e. binary representation:
// 0bWWRRGGBB for RGBW devices
// 0bRRRRGGBB for RGB

// RGB NeoPixel permutations; white and red offsets are always same
// Offset:         W          R          G          B
#define NEO_RGB  ((0 << 6) | (0 << 4) | (1 << 2) | (2))
#define NEO_RBG  ((0 << 6) | (0 << 4) | (2 << 2) | (1))
#define NEO_GRB  ((1 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_GBR  ((2 << 6) | (2 << 4) | (0 << 2) | (1))
#define NEO_BRG  ((1 << 6) | (1 << 4) | (2 << 2) | (0))
#define NEO_BGR  ((2 << 6) | (2 << 4) | (1 << 2) | (0))

// RGBW NeoPixel permutations; all 4 offsets are distinct
// Offset:         W          R          G          B
#define NEO_WRGB ((0 << 6) | (1 << 4) | (2 << 2) | (3))
#define NEO_WRBG ((0 << 6) | (1 << 4) | (3 << 2) | (2))
#define NEO_WGRB ((0 << 6) | (2 << 4) | (1 << 2) | (3))
#define NEO_WGBR ((0 << 6) | (3 << 4) | (1 << 2) | (2))
#define NEO_WBRG ((0 << 6) | (2 << 4) | (3 << 2) | (1))
#define NEO_WBGR ((0 << 6) | (3 << 4) | (2 << 2) | (1))

#define NEO_RWGB ((1 << 6) | (0 << 4) | (2 << 2) | (3))
#define NEO_RWBG ((1 << 6) | (0 << 4) | (3 << 2) | (2))
#define NEO_RGWB ((2 << 6) | (0 << 4) | (1 << 2) | (3))
#define NEO_RGBW ((3 << 6) | (0 << 4) | (1 << 2) | (2))
#define NEO_RBWG ((2 << 6) | (0 << 4) | (3 << 2) | (1))
#define NEO_RBGW ((3 << 6) | (0 << 4) | (2 << 2) | (1))

#define NEO_GWRB ((1 << 6) | (2 << 4) | (0 << 2) | (3))
#define NEO_GWBR ((1 << 6) | (3 << 4) | (0 << 2) | (2))
#define NEO_GRWB ((2 << 6) | (1 << 4) | (0 << 2) | (3))
#define NEO_GRBW ((3 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_GBWR ((2 << 6) | (3 << 4) | (0 << 2) | (1))
#define NEO_GBRW ((3 << 6) | (2 << 4) | (0 << 2) | (1))

#define NEO_BWRG ((1 << 6) | (2 << 4) | (3 << 2) | (0))
#define NEO_BWGR ((1 << 6) | (3 << 4) | (2 << 2) | (0))
#define NEO_BRWG ((2 << 6) | (1 << 4) | (3 << 2) | (0))
#define NEO_BRGW ((3 << 6) | (1 << 4) | (2 << 2) | (0))
#define NEO_BGWR ((2 << 6) | (3 << 4) | (1 << 2) | (0))
#define NEO_BGRW ((3 << 6) | (2 << 4) | (1 << 2) | (0))

// Add NEO_KHZ400 to the color order value to indicate a 400 KHz
// device.  All but the earliest v1 NeoPixels expect an 800 KHz data
// stream, this is the default if unspecified.  Because flash space
// is very limited on ATtiny devices (e.g. Trinket, Gemma), v1
// NeoPixels aren't handled by default on those chips, though it can
// be enabled by removing the ifndef/endif below -- but code will be
// bigger.  Conversely, can disable the NEO_KHZ400 line on other MCUs
// to remove v1 support and save a little space.

#define NEO_KHZ800 0x0000 // 800 KHz datastream
#ifndef __AVR_ATtiny85__
#define NEO_KHZ400 0x0100 // 400 KHz datastream
#endif

typedef uint16_t neoPixelType;

typedef struct {
	bool is800KHz;		// ...true if 800 KHz pixels
	bool begun;			// true if begin() previously called
	uint16_t numLEDs;	// Number of RGB LEDs in strip
	uint16_t numBytes;	// Size of 'pixels' buffer below (3 or 4 bytes/pixel)
	int8_t pin;			// Output pin number (-1 if not yet set)
	uint8_t brightness;
	uint8_t *pixels;		// Holds LED color values (3 or 4 bytes each)
	uint8_t rOffset;		// Index of red byte within each 3- or 4-byte pixel
	uint8_t gOffset;		// Index of green byte
	uint8_t bOffset;		// Index of blue byte
	uint8_t wOffset;		// Index of white byte (same as rOffset if no white)
	uint32_t endTime;	// Latch timing reference
	volatile uint8_t *port;// Output PORT register
	uint8_t pinMask;		// Output PORT bitmask
} Adafruit_NeoPixel;

  Adafruit_NeoPixel *createNeoPixel(uint16_t n, uint8_t p, neoPixelType t);
  Adafruit_NeoPixel *createNeoPixelNoParams(void);
  void freeNeoPixel(Adafruit_NeoPixel *adaNeo);

  void set_all_members_to_0(Adafruit_NeoPixel *adaNeo);
  void begin(Adafruit_NeoPixel *adaNeo);
  void show(Adafruit_NeoPixel *adaNeo);
  void setPin(Adafruit_NeoPixel *adaNeo, uint8_t p);
  void setPixelColorRGB(Adafruit_NeoPixel *adaNeo,uint16_t n, uint8_t r, uint8_t g, uint8_t b);
  void setPixelColorRGBW(Adafruit_NeoPixel *adaNeo,uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
  void setPixelColor32Bit(Adafruit_NeoPixel *adaNeo,uint16_t n, uint32_t c);
  void setBrightness(Adafruit_NeoPixel *adaNeo,uint8_t);
  void clear(Adafruit_NeoPixel *adaNeo);
  void updateLength(Adafruit_NeoPixel *adaNeo,uint16_t n);
  void updateType(Adafruit_NeoPixel *adaNeo,neoPixelType t);
  uint8_t *getPixels(Adafruit_NeoPixel *adaNeo);
  uint8_t getBrightness(Adafruit_NeoPixel *adaNeo);
  int8_t getPin(Adafruit_NeoPixel *adaNeo);
  uint16_t numPixels(Adafruit_NeoPixel *adaNeo);
  uint32_t ColorRGB(uint8_t r, uint8_t g, uint8_t b);
  uint32_t ColorRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
  uint32_t getPixelColor(Adafruit_NeoPixel *adaNeo,uint16_t n);
  bool canShow(Adafruit_NeoPixel *adaNeo);
#endif // ADAFRUIT_NEOPIXEL_H