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

#ifndef SSD1306Brzo_h
#define SSD1306Brzo_h

#include "OLEDDisplay.h"
#include <brzo_i2c.h>

#if F_CPU == 160000000L
  #define BRZO_I2C_SPEED 1000
#else
  #define BRZO_I2C_SPEED 800
#endif

class SSD1306Brzo : public OLEDDisplay {
  private:
      uint8_t             _address;
      uint8_t             _sda;
      uint8_t             _scl;

  public:
    SSD1306Brzo(uint8_t _address, uint8_t _sda, uint8_t _scl, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64) {
      setGeometry(g);

      this->_address = _address;
      this->_sda = _sda;
      this->_scl = _scl;
    }

    bool connect(){
      brzo_i2c_setup(_sda, _scl, 0);
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

       sendCommand(COLUMNADDR);
       sendCommand(minBoundX);
       sendCommand(maxBoundX);

       sendCommand(PAGEADDR);
       sendCommand(minBoundY);
       sendCommand(maxBoundY);

       byte k = 0;
       uint8_t sendBuffer[17];
       sendBuffer[0] = 0x40;
       brzo_i2c_start_transaction(this->_address, BRZO_I2C_SPEED);
       for (y = minBoundY; y <= maxBoundY; y++) {
           for (x = minBoundX; x <= maxBoundX; x++) {
               k++;
               sendBuffer[k] = buffer[x + y * displayWidth];
               if (k == 16)  {
                 brzo_i2c_write(sendBuffer, 17, true);
                 k = 0;
               }
           }
           yield();
       }
       brzo_i2c_write(sendBuffer, k + 1, true);
       brzo_i2c_end_transaction();
     #else
       // No double buffering
       sendCommand(COLUMNADDR);
       sendCommand(0x0);
       sendCommand(0x7F);

       sendCommand(PAGEADDR);
       sendCommand(0x0);

       if (geometry == GEOMETRY_128_64) {
         sendCommand(0x7);
       } else if (geometry == GEOMETRY_128_32) {
         sendCommand(0x3);
       }

       uint8_t sendBuffer[17];
       sendBuffer[0] = 0x40;
       brzo_i2c_start_transaction(this->_address, BRZO_I2C_SPEED);
       for (uint16_t i=0; i<displayBufferSize; i++) {
         for (uint8_t x=1; x<17; x++) {
           sendBuffer[x] = buffer[i];
           i++;
         }
         i--;
         brzo_i2c_write(sendBuffer,  17,  true);
         yield();
       }
       brzo_i2c_end_transaction();
     #endif
    }

  private:
	int getBufferOffset(void) {
		return 0;
	}
    inline void sendCommand(uint8_t com) __attribute__((always_inline)){
      uint8_t command[2] = {0x80 /* command mode */, com};
      brzo_i2c_start_transaction(_address, BRZO_I2C_SPEED);
      brzo_i2c_write(command, 2, true);
      brzo_i2c_end_transaction();
    }
};

#endif
