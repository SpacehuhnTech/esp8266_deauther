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

#ifndef SH1106Wire_h
#define SH1106Wire_h

#include "OLEDDisplay.h"
#include <Wire.h>

#define SH1106_SET_PUMP_VOLTAGE 0X30
#define SH1106_SET_PUMP_MODE 0XAD
#define SH1106_PUMP_ON 0X8B
#define SH1106_PUMP_OFF 0X8A
//--------------------------------------

class SH1106Wire : public OLEDDisplay {
  private:
      uint8_t             _address;
      uint8_t             _sda;
      uint8_t             _scl;

  public:
    SH1106Wire(uint8_t _address, uint8_t _sda, uint8_t _scl, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64) {
      setGeometry(g);

      this->_address = _address;
      this->_sda = _sda;
      this->_scl = _scl;
    }

    bool connect() {
      Wire.begin(this->_sda, this->_scl);
      // Let's use ~700khz if ESP8266 is in 160Mhz mode
      // this will be limited to ~400khz if the ESP8266 in 80Mhz mode.
      Wire.setClock(700000);
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

        byte k = 0;
        for (y = minBoundY; y <= maxBoundY; y++) {
          sendCommand(0xB0 + y);
          sendCommand(minBoundXp2H);
          sendCommand(minBoundXp2L);
          for (x = minBoundX; x <= maxBoundX; x++) {
            if (k == 0) {
              Wire.beginTransmission(_address);
              Wire.write(0x40);
            }
            Wire.write(buffer[x + y * displayWidth]);
            k++;
            if (k == 16)  {
              Wire.endTransmission();
              k = 0;
            }
          }
          if (k != 0)  {
            Wire.endTransmission();
            k = 0;
          }
          yield();
        }

        if (k != 0) {
          Wire.endTransmission();
        }
      #else
        uint8_t * p = &buffer[0];
        for (uint8_t y=0; y<8; y++) {
          sendCommand(0xB0+y);
          sendCommand(0x02);
          sendCommand(0x10);
          for( uint8_t x=0; x<8; x++) {
            Wire.beginTransmission(_address);
            Wire.write(0x40);
            for (uint8_t k = 0; k < 16; k++) {
              Wire.write(*p++);
            }
            Wire.endTransmission();
          }
        }
      #endif
    }

  private:
	int getBufferOffset(void) {
		return 0;
	}
    inline void sendCommand(uint8_t command) __attribute__((always_inline)){
      Wire.beginTransmission(_address);
      Wire.write(0x80);
      Wire.write(command);
      Wire.endTransmission();
    }


};

#endif
