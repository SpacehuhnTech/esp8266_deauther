/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 * Copyright (c) 2019 by Helmut Tschemernjak - www.radioshuttle.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#ifndef OLEDDISPLAY_h
#define OLEDDISPLAY_h

#ifdef ARDUINO
#include <Arduino.h>
#elif __MBED__
#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))

#include <mbed.h>
#define delay(x)	wait_ms(x)
#define yield()		void()

/*
 * This is a little Arduino String emulation to keep the OLEDDisplay
 * library code in common between Arduino and mbed-os
 */
class String {
public:
	String(const char *s) { _str = s; };
	int length() { return strlen(_str); };
	const char *c_str() { return _str; };
    void toCharArray(char *buf, unsigned int bufsize, unsigned int index = 0) const {
		memcpy(buf, _str + index,  std::min(bufsize, strlen(_str)));
	};
private:
	const char *_str;
};

#else
#error "Unkown operating system"
#endif

#include "OLEDDisplayFonts.h"

//#define DEBUG_OLEDDISPLAY(...) Serial.printf( __VA_ARGS__ )
//#define DEBUG_OLEDDISPLAY(...) dprintf("%s",  __VA_ARGS__ )

#ifndef DEBUG_OLEDDISPLAY
#define DEBUG_OLEDDISPLAY(...)
#endif

// Use DOUBLE BUFFERING by default
#ifndef OLEDDISPLAY_REDUCE_MEMORY
#define OLEDDISPLAY_DOUBLE_BUFFER
#endif

// Header Values
#define JUMPTABLE_BYTES 4

#define JUMPTABLE_LSB   1
#define JUMPTABLE_SIZE  2
#define JUMPTABLE_WIDTH 3
#define JUMPTABLE_START 4

#define WIDTH_POS 0
#define HEIGHT_POS 1
#define FIRST_CHAR_POS 2
#define CHAR_NUM_POS 3


// Display commands
#define CHARGEPUMP 0x8D
#define COLUMNADDR 0x21
#define COMSCANDEC 0xC8
#define COMSCANINC 0xC0
#define DISPLAYALLON 0xA5
#define DISPLAYALLON_RESUME 0xA4
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define EXTERNALVCC 0x1
#define INVERTDISPLAY 0xA7
#define MEMORYMODE 0x20
#define NORMALDISPLAY 0xA6
#define PAGEADDR 0x22
#define SEGREMAP 0xA0
#define SETCOMPINS 0xDA
#define SETCONTRAST 0x81
#define SETDISPLAYCLOCKDIV 0xD5
#define SETDISPLAYOFFSET 0xD3
#define SETHIGHCOLUMN 0x10
#define SETLOWCOLUMN 0x00
#define SETMULTIPLEX 0xA8
#define SETPRECHARGE 0xD9
#define SETSEGMENTREMAP 0xA1
#define SETSTARTLINE 0x40
#define SETVCOMDETECT 0xDB
#define SWITCHCAPVCC 0x2

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

enum OLEDDISPLAY_COLOR {
  BLACK = 0,
  WHITE = 1,
  INVERSE = 2
};

enum OLEDDISPLAY_TEXT_ALIGNMENT {
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_RIGHT = 1,
  TEXT_ALIGN_CENTER = 2,
  TEXT_ALIGN_CENTER_BOTH = 3
};


enum OLEDDISPLAY_GEOMETRY {
  GEOMETRY_128_64   = 0,
  GEOMETRY_128_32,
  GEOMETRY_RAWMODE,
};

typedef char (*FontTableLookupFunction)(const uint8_t ch);
char DefaultFontTableLookup(const uint8_t ch);


#ifdef ARDUINO
class OLEDDisplay : public Print  {
#elif __MBED__
class OLEDDisplay : public Stream {
#else
#error "Unkown operating system"
#endif

  public:
	OLEDDisplay();
    virtual ~OLEDDisplay();

	uint16_t width(void) const { return displayWidth; };
	uint16_t height(void) const { return displayHeight; };

    // Use this to resume after a deep sleep without resetting the display (what init() would do).
    // Returns true if connection to the display was established and the buffer allocated, false otherwise.
    bool allocateBuffer();

    // Allocates the buffer and initializes the driver & display. Resets the display!
    // Returns false if buffer allocation failed, true otherwise.
    bool init();

    // Free the memory used by the display
    void end();

    // Cycle through the initialization
    void resetDisplay(void);

    /* Drawing functions */
    // Sets the color of all pixel operations
    void setColor(OLEDDISPLAY_COLOR color);

    // Returns the current color.
    OLEDDISPLAY_COLOR getColor();

    // Draw a pixel at given position
    void setPixel(int16_t x, int16_t y);

    // Draw a pixel at given position and color
    void setPixelColor(int16_t x, int16_t y, OLEDDISPLAY_COLOR color);

    // Clear a pixel at given position FIXME: INVERSE is untested with this function
    void clearPixel(int16_t x, int16_t y);

    // Draw a line from position 0 to position 1
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

    // Draw the border of a rectangle at the given location
    void drawRect(int16_t x, int16_t y, int16_t width, int16_t height);

    // Fill the rectangle
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height);

    // Draw the border of a circle
    void drawCircle(int16_t x, int16_t y, int16_t radius);

    // Draw all Quadrants specified in the quads bit mask
    void drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads);

    // Fill circle
    void fillCircle(int16_t x, int16_t y, int16_t radius);

    // Draw a line horizontally
    void drawHorizontalLine(int16_t x, int16_t y, int16_t length);

    // Draw a line vertically
    void drawVerticalLine(int16_t x, int16_t y, int16_t length);

    // Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
    // a unsigned byte value between 0 and 100
    void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);

    // Draw a bitmap in the internal image format
    void drawFastImage(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *image);

    // Draw a XBM
    void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *xbm);

    // Draw icon 16x16 xbm format
    void drawIco16x16(int16_t x, int16_t y, const char *ico, bool inverse = false);

    /* Text functions */

    // Draws a string at the given location
    void drawString(int16_t x, int16_t y, String text);

    // Draws a String with a maximum width at the given location.
    // If the given String is wider than the specified width
    // The text will be wrapped to the next line at a space or dash
    void drawStringMaxWidth(int16_t x, int16_t y, uint16_t maxLineWidth, String text);

    // Returns the width of the const char* with the current
    // font settings
    uint16_t getStringWidth(const char* text, uint16_t length);

    // Convencience method for the const char version
    uint16_t getStringWidth(String text);

    // Specifies relative to which anchor point
    // the text is rendered. Available constants:
    // TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER_BOTH
    void setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment);

    // Sets the current font. Available default fonts
    // ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
    void setFont(const uint8_t *fontData);

    // Set the function that will convert utf-8 to font table index
    void setFontTableLookupFunction(FontTableLookupFunction function);

    /* Display functions */

    // Turn the display on
    void displayOn(void);

    // Turn the display offs
    void displayOff(void);

    // Inverted display mode
    void invertDisplay(void);

    // Normal display mode
    void normalDisplay(void);

    // Set display contrast
    // really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
    // normal brightness & contrast:  contrast = 100
    void setContrast(uint8_t contrast, uint8_t precharge = 241, uint8_t comdetect = 64);

    // Convenience method to access 
    void setBrightness(uint8_t);

    // Reset display rotation or mirroring
    void resetOrientation();

    // Turn the display upside down
    void flipScreenVertically();

    // Mirror the display (to be used in a mirror or as a projector)
    void mirrorScreen();

    // Write the buffer to the display memory
    virtual void display(void) = 0;

    // Clear the local pixel buffer
    void clear(void);

    // Log buffer implementation

    // This will define the lines and characters you can
    // print to the screen. When you exeed the buffer size (lines * chars)
    // the output may be truncated due to the size constraint.
    bool setLogBuffer(uint16_t lines, uint16_t chars);

    // Draw the log buffer at position (x, y)
    void drawLogBuffer(uint16_t x, uint16_t y);

    // Get screen geometry
    uint16_t getWidth(void);
    uint16_t getHeight(void);

    // Implement needed function to be compatible with Print class
    size_t write(uint8_t c);
    size_t write(const char* s);
	
    // Implement needed function to be compatible with Stream class
#ifdef __MBED__
	int _putc(int c);
	int _getc() { return -1; };
#endif


    uint8_t            *buffer;

    #ifdef OLEDDISPLAY_DOUBLE_BUFFER
    uint8_t            *buffer_back;
    #endif

  protected:

    OLEDDISPLAY_GEOMETRY geometry;

    uint16_t  displayWidth;
    uint16_t  displayHeight;
    uint16_t  displayBufferSize;

    // Set the correct height, width and buffer for the geometry
    void setGeometry(OLEDDISPLAY_GEOMETRY g, uint16_t width = 0, uint16_t height = 0);

    OLEDDISPLAY_TEXT_ALIGNMENT   textAlignment;
    OLEDDISPLAY_COLOR            color;

    const uint8_t	 *fontData;

    // State values for logBuffer
    uint16_t   logBufferSize;
    uint16_t   logBufferFilled;
    uint16_t   logBufferLine;
    uint16_t   logBufferMaxLines;
    char      *logBuffer;


	// the header size of the buffer used, e.g. for the SPI command header
	virtual int getBufferOffset(void) = 0;
	
    // Send a command to the display (low level function)
    virtual void sendCommand(uint8_t com) {(void)com;};

    // Connect to the display
    virtual bool connect() { return false; };

    // Send all the init commands
    void sendInitCommands();

    // converts utf8 characters to extended ascii
    char* utf8ascii(String s);

    void inline drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const uint8_t *data, uint16_t offset, uint16_t bytesInData) __attribute__((always_inline));

    void drawStringInternal(int16_t xMove, int16_t yMove, char* text, uint16_t textLength, uint16_t textWidth);
	
	FontTableLookupFunction fontTableLookupFunction;
};

#endif
