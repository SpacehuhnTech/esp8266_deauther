/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
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

#ifndef SH1106Spi_h
#define SH1106Spi_h

#include "OLEDDisplay.h"
#include <SPI.h>

class SH1106Spi : public OLEDDisplay {
  private:
      uint8_t             _rst;
      uint8_t             _dc;

  public:
    SH1106Spi(uint8_t _rst, uint8_t _dc, uint8_t _cs, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64) {
        setGeometry(g);

      this->_rst = _rst;
      this->_dc  = _dc;
    }

    bool connect(){
      pinMode(_dc, OUTPUT);
      pinMode(_rst, OUTPUT);

      SPI.begin ();
      SPI.setClockDivider (SPI_CLOCK_DIV2);

      // Pulse Reset low for 10ms
      digitalWrite(_rst, HIGH);
      delay(1);
      digitalWrite(_rst, LOW);
      delay(10);
      digitalWrite(_rst, HIGH);
      return true;
    }

    void display(void) {
    #ifdef OLEDDISPLAY_DOUBLE_BUFFER
       uint8_t minBoundY = UINT8_MAX;
       uint8_t maxBoundY = 0;

       uint8_t minBoundX = UINT8_MAX;
       uint8_t maxBoundX = 0;

       uint8_t x, y;

       // Calculate the Y bounding box of changes
       // and copy buffer[pos] to buffer_back[pos];
       for (y = 0; y < (displayHeight / 8); y++) {
         for (x = 0; x < displayWidth; x++) {
          uint16_t pos = x + y * displayWidth;
          if (buffer[pos] != buffer_back[pos]) {
            minBoundY = _min(minBoundY, y);
            maxBoundY = _max(maxBoundY, y);
            minBoundX = _min(minBoundX, x);
            maxBoundX = _max(maxBoundX, x);
          }
          buffer_back[pos] = buffer[pos];
        }
        yield();
       }

       // If the minBoundY wasn't updated
       // we can savely assume that buffer_back[pos] == buffer[pos]
       // holdes true for all values of pos
       if (minBoundY == UINT8_MAX) return;

       // Calculate the colum offset
       uint8_t minBoundXp2H = (minBoundX + 2) & 0x0F;
       uint8_t minBoundXp2L = 0x10 | ((minBoundX + 2) >> 4 );

       for (y = minBoundY; y <= maxBoundY; y++) {
         sendCommand(0xB0 + y);
         sendCommand(minBoundXp2H);
         sendCommand(minBoundXp2L);
         digitalWrite(_dc, HIGH);   // data mode
         for (x = minBoundX; x <= maxBoundX; x++) {
           SPI.transfer(buffer[x + y * displayWidth]);
         }
         yield();
       }
     #else
      for (uint8_t y=0; y<displayHeight/8; y++) {
        sendCommand(0xB0 + y);
        sendCommand(0x02);
        sendCommand(0x10);
        digitalWrite(_dc, HIGH);   // data mode
        for( uint8_t x=0; x < displayWidth; x++) {
          SPI.transfer(buffer[x + y * displayWidth]);
        }
        yield();
      }
     #endif
    }

  private:
	int getBufferOffset(void) {
		return 0;
	}
    inline void sendCommand(uint8_t com) __attribute__((always_inline)){
      digitalWrite(_dc, LOW);
      SPI.transfer(com);
    }
};

#endif
