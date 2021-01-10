/*!
 * @file Adafruit_DotStar.cpp
 *
 * @mainpage Arduino Library for driving Adafruit DotStar addressable LEDs
 * and compatible devicess -- APA102, etc.
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's DotStar library for the
 * Arduino platform, allowing a broad range of microcontroller boards
 * (most AVR boards, many ARM devices, ESP8266 and ESP32, among others)
 * to control Adafruit DotStars and compatible devices -- APA102, etc.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing products
 * from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried and Phil Burgess for Adafruit Industries with
 * contributions from members of the open source community.
 *
 * @section license License
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

#include "Adafruit_DotStar.h"
#if !defined(__AVR_ATtiny85__)
#include <SPI.h>
#endif

#define USE_HW_SPI 255 ///< Assigned to dataPin to indicate 'hard' SPI

/*!
  @brief   DotStar constructor for hardware SPI. Must be connected to
           MOSI, SCK pins.
  @param   n  Number of DotStars in strand.
  @param   o  Pixel type -- one of the DOTSTAR_* constants defined in
              Adafruit_DotStar.h, for example DOTSTAR_BRG for DotStars
              expecting color bytes expressed in blue, red, green order
              per pixel. Default if unspecified is DOTSTAR_BRG.
  @return  Adafruit_DotStar object. Call the begin() function before use.
*/
Adafruit_DotStar::Adafruit_DotStar(uint16_t n, uint8_t o)
    : numLEDs(n), dataPin(USE_HW_SPI), brightness(0), pixels(NULL),
      rOffset(o & 3), gOffset((o >> 2) & 3), bOffset((o >> 4) & 3) {
  updateLength(n);
}

/*!
  @brief   DotStar constructor for 'soft' (bitbang) SPI. Any two pins
           can be used.
  @param   n      Number of DotStars in strand.
  @param   data   Arduino pin number for data out.
  @param   clock  Arduino pin number for clock out.
  @param   o      Pixel type -- one of the DOTSTAR_* constants defined in
                  Adafruit_DotStar.h, for example DOTSTAR_BRG for DotStars
                  expecting color bytes expressed in blue, red, green order
                  per pixel. Default if unspecified is DOTSTAR_BRG.
  @return  Adafruit_DotStar object. Call the begin() function before use.
*/
Adafruit_DotStar::Adafruit_DotStar(uint16_t n, uint8_t data, uint8_t clock,
                                   uint8_t o)
    : dataPin(data), clockPin(clock), brightness(0), pixels(NULL),
      rOffset(o & 3), gOffset((o >> 2) & 3), bOffset((o >> 4) & 3) {
  updateLength(n);
}

/*!
  @brief   Deallocate Adafruit_DotStar object, set data and clock pins
           back to INPUT.
*/
Adafruit_DotStar::~Adafruit_DotStar(void) {
  free(pixels);
  if (dataPin == USE_HW_SPI)
    hw_spi_end();
  else
    sw_spi_end();
}

/*!
  @brief   Initialize Adafruit_DotStar object -- sets data and clock pins
           to outputs and initializes hardware SPI if necessary.
*/
void Adafruit_DotStar::begin(void) {
  if (dataPin == USE_HW_SPI)
    hw_spi_init();
  else
    sw_spi_init();
}

// Pins may be reassigned post-begin(), so a sketch can store hardware
// config in flash, SD card, etc. rather than hardcoded. Also permits
// "recycling" LED ram across multiple strips: set pins to first strip,
// render & write all data, reassign pins to next strip, render & write,
// etc. They won't update simultaneously, but usually unnoticeable.

/*!
  @brief   Switch over to hardware SPI. DotStars must be connected to
           MOSI, SCK pins. Data in pixel buffer is unaffected and can
           continue to be used.
*/
void Adafruit_DotStar::updatePins(void) {
  sw_spi_end();
  dataPin = USE_HW_SPI;
  hw_spi_init();
}

/*!
  @brief   Switch over to 'soft' (bitbang) SPI. DotStars can be connected
           to any two pins. Data in pixel buffer is unaffected and can
           continue to be used.
  @param   data   Arduino pin number for data out.
  @param   clock  Arduino pin number for clock out.
*/
void Adafruit_DotStar::updatePins(uint8_t data, uint8_t clock) {
  hw_spi_end();
  dataPin = data;
  clockPin = clock;
  sw_spi_init();
}

/*!
  @brief   Change the length of a previously-declared Adafruit_DotStar
           strip object. Old data is deallocated and new data is cleared.
           Pin numbers and pixel format are unchanged.
  @param   n  New length of strip, in pixels.
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword.
*/
void Adafruit_DotStar::updateLength(uint16_t n) {
  free(pixels);
  uint16_t bytes = (rOffset == gOffset)
                       ? n + ((n + 3) / 4)
                       :      // MONO: 10 bits/pixel, round up to next byte
                       n * 3; // COLOR: 3 bytes/pixel
  if ((pixels = (uint8_t *)malloc(bytes))) {
    numLEDs = n;
    clear();
  } else {
    numLEDs = 0;
  }
}

// SPI STUFF ---------------------------------------------------------------

/*!
  @brief   Initialize hardware SPI.
  @note    This library is written in pre-SPI-transactions style and needs
           some rewriting to correctly share the SPI bus with other devices.
*/
void Adafruit_DotStar::hw_spi_init(void) { // Initialize hardware SPI
#ifdef __AVR_ATtiny85__
  PORTB &= ~(_BV(PORTB1) | _BV(PORTB2)); // Outputs
  DDRB |= _BV(PORTB1) | _BV(PORTB2);     // DO (NOT MOSI) + SCK
#elif (SPI_INTERFACES_COUNT > 0) || !defined(SPI_INTERFACES_COUNT)
  SPI.begin();
  // Hardware SPI clock speeds are chosen to run at roughly 1-8 MHz for most
  // boards, providing a slower but more reliable experience by default.  If
  // you want faster LED updates, experiment with the clock speeds to find
  // what works best with your particular setup.
#if defined(__AVR__) || defined(CORE_TEENSY) || defined(__ARDUINO_ARC__) ||    \
    defined(__ARDUINO_X86__)
  SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (6 MHz on Pro Trinket 3V)
#else
#ifdef ESP8266
  SPI.setFrequency(8000000L);
#elif defined(PIC32)
  // Use begin/end transaction to set SPI clock rate
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  SPI.endTransaction();
#else
  SPI.setClockDivider((F_CPU + 4000000L) / 8000000L); // 8-ish MHz on Due
#endif
#endif
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
#endif
}

/*!
  @brief   Stop hardware SPI.
*/
void Adafruit_DotStar::hw_spi_end(void) {
#ifdef __AVR_ATtiny85__
  DDRB &= ~(_BV(PORTB1) | _BV(PORTB2)); // Inputs
#elif (SPI_INTERFACES_COUNT > 0) || !defined(SPI_INTERFACES_COUNT)
  SPI.end();
#endif
}

/*!
  @brief   Initialize 'soft' (bitbang) SPI. Data and clock pins are set
           to outputs.
*/
void Adafruit_DotStar::sw_spi_init(void) {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
#ifdef __AVR__
  dataPort = portOutputRegister(digitalPinToPort(dataPin));
  clockPort = portOutputRegister(digitalPinToPort(clockPin));
  dataPinMask = digitalPinToBitMask(dataPin);
  clockPinMask = digitalPinToBitMask(clockPin);
  *dataPort &= ~dataPinMask;
  *clockPort &= ~clockPinMask;
#else
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
#endif
}

/*!
  @brief   Stop 'soft' (bitbang) SPI. Data and clock pins are set to inputs.
*/
void Adafruit_DotStar::sw_spi_end() {
  pinMode(dataPin, INPUT);
  pinMode(clockPin, INPUT);
}

#ifdef __AVR_ATtiny85__

// Teensy/Gemma-specific stuff for hardware-half-assisted SPI

#define SPIBIT                                                                 \
  USICR = ((1 << USIWM0) | (1 << USITC));                                      \
  USICR =                                                                      \
      ((1 << USIWM0) | (1 << USITC) | (1 << USICLK)); // Clock bit tick, tock

static void spi_out(uint8_t n) { // Clock out one byte
  USIDR = n;
  SPIBIT SPIBIT SPIBIT SPIBIT SPIBIT SPIBIT SPIBIT SPIBIT
}

#elif (SPI_INTERFACES_COUNT > 0) || !defined(SPI_INTERFACES_COUNT)

// All other boards have full-featured hardware support for SPI

#define spi_out(n) (void)SPI.transfer(n) ///< Call hardware SPI function
// Pipelining reads next byte while current byte is clocked out
#if (defined(__AVR__) && !defined(__AVR_ATtiny85__)) || defined(CORE_TEENSY)
#define SPI_PIPELINE
#endif

#else // no hardware spi
#define spi_out(n) sw_spi_out(n)

#endif

/*!
  @brief   Soft (bitbang) SPI write.
  @param   n  8-bit value to transfer.
*/
void Adafruit_DotStar::sw_spi_out(uint8_t n) {
  for (uint8_t i = 8; i--; n <<= 1) {
#ifdef __AVR__
    if (n & 0x80)
      *dataPort |= dataPinMask;
    else
      *dataPort &= ~dataPinMask;
    *clockPort |= clockPinMask;
    *clockPort &= ~clockPinMask;
#else
    if (n & 0x80)
      digitalWrite(dataPin, HIGH);
    else
      digitalWrite(dataPin, LOW);
    digitalWrite(clockPin, HIGH);
#if F_CPU >= 48000000
    __asm__ volatile("nop \n nop");
#endif
    digitalWrite(clockPin, LOW);
#if F_CPU >= 48000000
    __asm__ volatile("nop \n nop");
#endif
#endif
  }
}

/* ISSUE DATA TO LED STRIP -------------------------------------------------

  Although the LED driver has an additional per-pixel 5-bit brightness
  setting, it is NOT used or supported here. On APA102, the normally
  very fast PWM is gated through a much slower PWM (about 400 Hz),
  rendering it useless for POV or other high-speed things that are
  probably why one is using DotStars instead of NeoPixels in the first
  place. I'm told that some APA102 clones use current control rather than
  PWM for this, which would be much more worthwhile. Still, no support
  here, no plans for it. If you really can't live without it, you can fork
  the library and add it for your own use, but any pull requests for this
  are unlikely be merged for the foreseeable future.
*/

/*!
  @brief   Transmit pixel data in RAM to DotStars.
*/
void Adafruit_DotStar::show(void) {

  if (!pixels)
    return;

  uint8_t *ptr = pixels, i;            // -> LED data
  uint16_t n = numLEDs;                // Counter
  uint16_t b16 = (uint16_t)brightness; // Type-convert for fixed-point math

  if (dataPin == USE_HW_SPI) {

    // TO DO: modernize this for SPI transactions

#ifdef SPI_PIPELINE
    uint8_t next;
    for (i = 0; i < 3; i++)
      spi_out(0x00); // First 3 start-frame bytes
    SPDR = 0x00;     // 4th is pipelined
    do {             // For each pixel...
      while (!(SPSR & _BV(SPIF)))
        ;                       //  Wait for prior byte out
      SPDR = 0xFF;              //  Pixel start
      for (i = 0; i < 3; i++) { //  For R,G,B...
        next = brightness ? (*ptr++ * b16) >> 8 : *ptr++; // Read, scale
        while (!(SPSR & _BV(SPIF)))
          ;          //   Wait for prior byte out
        SPDR = next; //   Write scaled color
      }
    } while (--n);
    while (!(SPSR & _BV(SPIF)))
      ; // Wait for last byte out
#else
    for (i = 0; i < 4; i++)
      spi_out(0x00);   // 4 byte start-frame marker
    if (brightness) {  // Scale pixel brightness on output
      do {             // For each pixel...
        spi_out(0xFF); //  Pixel start
        for (i = 0; i < 3; i++)
          spi_out((*ptr++ * b16) >> 8); // Scale, write RGB
      } while (--n);
    } else {           // Full brightness (no scaling)
      do {             // For each pixel...
        spi_out(0xFF); //  Pixel start
        for (i = 0; i < 3; i++)
          spi_out(*ptr++); // Write R,G,B
      } while (--n);
    }
#endif
    // Four end-frame bytes are seemingly indistinguishable from a white
    // pixel, and empirical testing suggests it can be left out...but it's
    // always a good idea to follow the datasheet, in case future hardware
    // revisions are more strict (e.g. might mandate use of end-frame
    // before start-frame marker). i.e. let's not remove this. But after
    // testing a bit more the suggestion is to use at least (numLeds+1)/2
    // high values (1) or (numLeds+15)/16 full bytes as EndFrame. For details
    // see also:
    // https://cpldcpu.wordpress.com/2014/11/30/understanding-the-apa102-superled/
    for (i = 0; i < ((numLEDs + 15) / 16); i++)
      spi_out(0xFF);

  } else { // Soft (bitbang) SPI

    for (i = 0; i < 4; i++)
      sw_spi_out(0);      // Start-frame marker
    if (brightness) {     // Scale pixel brightness on output
      do {                // For each pixel...
        sw_spi_out(0xFF); //  Pixel start
        for (i = 0; i < 3; i++)
          sw_spi_out((*ptr++ * b16) >> 8); // Scale, write
      } while (--n);
    } else {              // Full brightness (no scaling)
      do {                // For each pixel...
        sw_spi_out(0xFF); //  Pixel start
        for (i = 0; i < 3; i++)
          sw_spi_out(*ptr++); // R,G,B
      } while (--n);
    }
    for (i = 0; i < ((numLEDs + 15) / 16); i++)
      sw_spi_out(0xFF); // End-frame marker (see note above)
  }
}

/*!
  @brief   Fill the whole DotStar strip with 0 / black / off.
*/
void Adafruit_DotStar::clear() {
  memset(pixels, 0,
         (rOffset == gOffset) ? numLEDs + ((numLEDs + 3) / 4)
                              : // MONO: 10 bits/pixel
             numLEDs * 3);      // COLOR: 3 bytes/pixel
}

/*!
  @brief   Set a pixel's color using separate red, green and blue components.
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
*/
void Adafruit_DotStar::setPixelColor(uint16_t n, uint8_t r, uint8_t g,
                                     uint8_t b) {
  if (n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

/*!
  @brief   Set a pixel's color using a 32-bit 'packed' RGB value.
  @param   n  Pixel index, starting from 0.
  @param   c  32-bit color value. Most significant byte is 0, second is
              red, then green, and least significant byte is blue.
              e.g. 0x00RRGGBB
*/
void Adafruit_DotStar::setPixelColor(uint16_t n, uint32_t c) {
  if (n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    p[rOffset] = (uint8_t)(c >> 16);
    p[gOffset] = (uint8_t)(c >> 8);
    p[bOffset] = (uint8_t)c;
  }
}

/*!
  @brief   Fill all or part of the DotStar strip with a color.
  @param   c      32-bit color value. Most significant byte is 0, second
                  is red, then green, and least significant byte is blue.
                  e.g. 0x00RRGGBB. If all arguments are unspecified, this
                  will be 0 (off).
  @param   first  Index of first pixel to fill, starting from 0. Must be
                  in-bounds, no clipping is performed. 0 if unspecified.
  @param   count  Number of pixels to fill, as a positive value. Passing
                  0 or leaving unspecified will fill to end of strip.
*/
void Adafruit_DotStar::fill(uint32_t c, uint16_t first, uint16_t count) {
  uint16_t i, end;

  if (first >= numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if (count == 0) {
    // Fill to end of strip
    end = numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if (end > numLEDs)
      end = numLEDs;
  }

  for (i = first; i < end; i++) {
    this->setPixelColor(i, c);
  }
}

/*!
  @brief   Convert hue, saturation and value into a packed 32-bit RGB color
           that can be passed to setPixelColor() or other RGB-compatible
           functions.
  @param   hue  An unsigned 16-bit value, 0 to 65535, representing one full
                loop of the color wheel, which allows 16-bit hues to "roll
                over" while still doing the expected thing (and allowing
                more precision than the wheel() function that was common to
                prior DotStar and NeoPixel examples).
  @param   sat  Saturation, 8-bit value, 0 (min or pure grayscale) to 255
                (max or pure hue). Default of 255 if unspecified.
  @param   val  Value (brightness), 8-bit value, 0 (min / black / off) to
                255 (max or full brightness). Default of 255 if unspecified.
  @return  Packed 32-bit RGB color. Result is linearly but not perceptually
           correct, so you may want to pass the result through the gamma32()
           function (or your own gamma-correction operation) else colors may
           appear washed out. This is not done automatically by this
           function because coders may desire a more refined gamma-
           correction function than the simplified one-size-fits-all
           operation of gamma32(). Diffusing the LEDs also really seems to
           help when using low-saturation colors.
*/
uint32_t Adafruit_DotStar::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {

  uint8_t r, g, b;

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if (hue < 510) { // Red to Green-1
    b = 0;
    if (hue < 255) { //   Red to Yellow-1
      r = 255;
      g = hue;       //     g = 0 to 254
    } else {         //   Yellow to Green-1
      r = 510 - hue; //     r = 255 to 1
      g = 255;
    }
  } else if (hue < 1020) { // Green to Blue-1
    r = 0;
    if (hue < 765) { //   Green to Cyan-1
      g = 255;
      b = hue - 510;  //     b = 0 to 254
    } else {          //   Cyan to Blue-1
      g = 1020 - hue; //     g = 255 to 1
      b = 255;
    }
  } else if (hue < 1530) { // Blue to Red-1
    g = 0;
    if (hue < 1275) { //   Blue to Magenta-1
      r = hue - 1020; //     r = 0 to 254
      b = 255;
    } else { //   Magenta to Red-1
      r = 255;
      b = 1530 - hue; //     b = 255 to 1
    }
  } else { // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 = 1 + val;  // 1 to 256; allows >>8 instead of /255
  uint16_t s1 = 1 + sat;  // 1 to 256; same reason
  uint8_t s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
         (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
         (((((b * s1) >> 8) + s2) * v1) >> 8);
}

/*!
  @brief   Query the color of a previously-set pixel.
  @param   n  Index of pixel to read (0 = first).
  @return  'Packed' 32-bit RGB value. Most significant byte is 0, second is
           is red, then green, and least significant byte is blue.
*/
uint32_t Adafruit_DotStar::getPixelColor(uint16_t n) const {
  if (n >= numLEDs)
    return 0;
  uint8_t *p = &pixels[n * 3];
  return ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) |
         (uint32_t)p[bOffset];
}

/*!
  @brief   Adjust output brightness. Does not immediately affect what's
           currently displayed on the LEDs. The next call to show() will
           refresh the LEDs at this level.
  @param   b  Brightness setting, 0=minimum (off), 255=brightest.
  @note    For various reasons I think brightness is better handled in
           one's sketch, but it's here for parity with the NeoPixel
           library. Good news is that brightness setting in this library
           is 'non destructive' -- it's applied as color data is being
           issued to the strip, not during setPixelColor(), and also
           means that getPixelColor() returns the exact value originally
           stored.
*/
void Adafruit_DotStar::setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed. This
  // optimizes the actual scaling math later, allowing a fast 8x8-bit
  // multiply and taking the MSB. 'brightness' is a uint8_t, adding 1
  // here may (intentionally) roll over...so 0 = max brightness (color
  // values are interpreted literally; no scaling), 1 = min brightness
  // (off), 255 = just below max brightness.
  brightness = b + 1;
}

/*!
  @brief   Retrieve the last-set brightness value for the strip.
  @return  Brightness value: 0 = minimum (off), 255 = maximum.
*/
uint8_t Adafruit_DotStar::getBrightness(void) const {
  return brightness - 1; // Reverse above operation
}

/*!
  @brief   A gamma-correction function for 32-bit packed RGB colors.
           Makes color transitions appear more perceptially correct.
  @param   x  32-bit packed RGB color.
  @return  Gamma-adjusted packed color, can then be passed in one of the
           setPixelColor() functions. Like gamma8(), this uses a fixed
           gamma correction exponent of 2.6, which seems reasonably okay
           for average DotStars in average tasks. If you need finer
           control you'll need to provide your own gamma-correction
           function instead.
*/
uint32_t Adafruit_DotStar::gamma32(uint32_t x) {
  uint8_t *y = (uint8_t *)&x;
  // All four bytes of a 32-bit value are filtered to avoid a bunch of
  // shifting and masking that would be necessary for properly handling
  // different endianisms (and each byte is a fairly trivial operation,
  // so it might not even be wasting cycles vs a check and branch.
  // In theory this might cause trouble *if* someone's storing information
  // in the unused most significant byte of an RGB value, but this seems
  // exceedingly rare and if it's encountered in reality they can mask
  // values going in or coming out.
  for (uint8_t i = 0; i < 4; i++)
    y[i] = gamma8(y[i]);
  return x; // Packed 32-bit return
}
