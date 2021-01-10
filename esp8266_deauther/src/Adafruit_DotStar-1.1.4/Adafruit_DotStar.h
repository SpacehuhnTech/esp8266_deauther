/*!
 * @file Adafruit_DotStar.h
 *
 * This file is part of the Adafruit_DotStar library.
 *
 * Adafruit_DotStar is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Adafruit_DotStar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with DotStar. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ADAFRUIT_DOT_STAR_H_
#define _ADAFRUIT_DOT_STAR_H_

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#include <pins_arduino.h>
#endif

// Color-order flag for LED pixels (optional extra parameter to constructor):
// Bits 0,1 = R index (0-2), bits 2,3 = G index, bits 4,5 = B index
#define DOTSTAR_RGB (0 | (1 << 2) | (2 << 4)) ///< Transmit as R,G,B
#define DOTSTAR_RBG (0 | (2 << 2) | (1 << 4)) ///< Transmit as R,B,G
#define DOTSTAR_GRB (1 | (0 << 2) | (2 << 4)) ///< Transmit as G,R,B
#define DOTSTAR_GBR (2 | (0 << 2) | (1 << 4)) ///< Transmit as G,B,R
#define DOTSTAR_BRG (1 | (2 << 2) | (0 << 4)) ///< Transmit as B,R,G
#define DOTSTAR_BGR (2 | (1 << 2) | (0 << 4)) ///< Transmit as B,G,R
#define DOTSTAR_MONO 0 ///< Single-color strip WIP DO NOT USE, use RGB for now

// These two tables are declared outside the Adafruit_DotStar class
// because some boards may require oldschool compilers that don't
// handle the C++11 constexpr keyword.

/* A PROGMEM (flash mem) table containing 8-bit unsigned sine wave (0-255).
   Copy & paste this snippet into a Python REPL to regenerate:
import math
for x in range(256):
    print("{:3},".format(int((math.sin(x/128.0*math.pi)+1.0)*127.5+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _DotStarSineTable[256] = {
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170,
    173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211,
    213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240,
    241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254,
    254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251,
    250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232,
    230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198,
    196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155,
    152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109,
    106, 103, 100, 97,  93,  90,  88,  85,  82,  79,  76,  73,  70,  67,  65,
    62,  59,  57,  54,  52,  49,  47,  44,  42,  40,  37,  35,  33,  31,  29,
    27,  25,  23,  21,  20,  18,  17,  15,  14,  12,  11,  10,  9,   7,   6,
    5,   5,   4,   3,   2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,
    0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,   10,  11,
    12,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,  37,
    40,  42,  44,  47,  49,  52,  54,  57,  59,  62,  65,  67,  70,  73,  76,
    79,  82,  85,  88,  90,  93,  97,  100, 103, 106, 109, 112, 115, 118, 121,
    124};

/* Similar to above, but for an 8-bit gamma-correction table.
   Copy & paste this snippet into a Python REPL to regenerate:
import math
gamma=2.6
for x in range(256):
    print("{:3},".format(int(math.pow((x)/255.0,gamma)*255.0+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _DotStarGammaTable[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,
    3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,
    11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,
    17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
    25,  26,  27,  27,  28,  29,  29,  30,  31,  31,  32,  33,  34,  34,  35,
    36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,  80,  81,
    82,  84,  85,  86,  88,  89,  90,  92,  93,  94,  96,  97,  99,  100, 102,
    103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
    127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148, 150, 152,
    154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182,
    184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
    218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252,
    255};

/*!
  @brief  Class that stores state and functions for interacting with
          Adafruit DotStars and compatible devices.
*/
class Adafruit_DotStar {

public:
  Adafruit_DotStar(uint16_t n, uint8_t o = DOTSTAR_BRG);
  Adafruit_DotStar(uint16_t n, uint8_t d, uint8_t c, uint8_t o = DOTSTAR_BRG);
  ~Adafruit_DotStar(void);

  void begin(void);
  void show(void);
  void setPixelColor(uint16_t n, uint32_t c);
  void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
  void fill(uint32_t c = 0, uint16_t first = 0, uint16_t count = 0);
  void setBrightness(uint8_t);
  void clear();
  void updateLength(uint16_t n);
  void updatePins(void);
  void updatePins(uint8_t d, uint8_t c);
  /*!
    @brief   Get a pointer directly to the DotStar data buffer in RAM.
             Pixel data is stored in a device-native format (a la the
             DOTSTAR_* constants) and is not translated here. Applications
             that access this buffer will need to be aware of the specific
             data format and handle colors appropriately.
    @return  Pointer to DotStar buffer (uint8_t* array).
    @note    This is for high-performance applications where calling
             setPixelColor() on every single pixel would be too slow (e.g.
             POV or light-painting projects). There is no bounds checking
             on the array, creating tremendous potential for mayhem if one
             writes past the ends of the buffer. Great power, great
             responsibility and all that.
  */
  uint8_t *getPixels(void) const { return pixels; };
  uint8_t getBrightness(void) const;
  /*!
    @brief   Return the number of pixels in an Adafruit_DotStar strip object.
    @return  Pixel count (0 if not set).
  */
  uint16_t numPixels(void) const { return numLEDs; };
  uint32_t getPixelColor(uint16_t n) const;
  /*!
    @brief   An 8-bit integer sine wave function, not directly compatible
             with standard trigonometric units like radians or degrees.
    @param   x  Input angle, 0-255; 256 would loop back to zero, completing
                the circle (equivalent to 360 degrees or 2 pi radians).
                One can therefore use an unsigned 8-bit variable and simply
                add or subtract, allowing it to overflow/underflow and it
                still does the expected contiguous thing.
    @return  Sine result, 0 to 255, or -128 to +127 if type-converted to
             a signed int8_t, but you'll most likely want unsigned as this
             output is often used for pixel brightness in animation effects.
  */
  static uint8_t sine8(uint8_t x) {
    return pgm_read_byte(&_DotStarSineTable[x]); // 0-255 in, 0-255 out
  }
  /*!
    @brief   An 8-bit gamma-correction function for basic pixel brightness
             adjustment. Makes color transitions appear more perceptially
             correct.
    @param   x  Input brightness, 0 (minimum or off/black) to 255 (maximum).
    @return  Gamma-adjusted brightness, can then be passed to one of the
             setPixelColor() functions. This uses a fixed gamma correction
             exponent of 2.6, which seems reasonably okay for average
             DotStars in average tasks. If you need finer control you'll
             need to provide your own gamma-correction function instead.
  */
  static uint8_t gamma8(uint8_t x) {
    return pgm_read_byte(&_DotStarGammaTable[x]); // 0-255 in, 0-255 out
  }
  /*!
    @brief   Convert separate red, green and blue values into a single
             "packed" 32-bit RGB color.
    @param   r  Red brightness, 0 to 255.
    @param   g  Green brightness, 0 to 255.
    @param   b  Blue brightness, 0 to 255.
    @return  32-bit packed RGB value, which can then be assigned to a
             variable for later use or passed to the setPixelColor()
             function. Packed RGB format is predictable, regardless of
             LED strand color order.
  */
  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }
  static uint32_t ColorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255);
  static uint32_t gamma32(uint32_t x);

private:
  uint16_t numLEDs;   ///< Number of pixels
  uint8_t dataPin;    ///< If soft SPI, data pin #
  uint8_t clockPin;   ///< If soft SPI, clock pin #
  uint8_t brightness; ///< Global brightness setting
  uint8_t *pixels;    ///< LED RGB values (3 bytes ea.)
  uint8_t rOffset;    ///< Index of red in 3-byte pixel
  uint8_t gOffset;    ///< Index of green byte
  uint8_t bOffset;    ///< Index of blue byte
#ifdef __AVR__
  uint8_t dataPinMask;         ///< If soft SPI, data pin bitmask
  uint8_t clockPinMask;        ///< If soft SPI, clock pin bitmask
  volatile uint8_t *dataPort;  ///< If soft SPI, data PORT
  volatile uint8_t *clockPort; ///< If soft SPI, clock PORT
#endif
  void hw_spi_init(void);     ///< Start hardware SPI
  void hw_spi_end(void);      ///< Stop hardware SPI
  void sw_spi_init(void);     ///< Start bitbang SPI
  void sw_spi_out(uint8_t n); ///< Bitbang SPI write
  void sw_spi_end(void);      ///< Stop bitbang SPI
};

#endif // _ADAFRUIT_DOT_STAR_H_
