/**
 * The MIT License (MIT)
 *
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

#ifndef SSD1306I2C_h
#define SSD1306I2C_h


#ifdef __MBED__

#include "OLEDDisplay.h"
#include <mbed.h>

#ifndef UINT8_MAX
 #define UINT8_MAX 0xff
#endif

class SSD1306I2C : public OLEDDisplay {
public:
    SSD1306I2C(uint8_t _address, PinName _sda, PinName _scl, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64) {
      setGeometry(g);

      this->_address = _address << 1;  // convert from 7 to 8 bit for mbed.
      this->_sda = _sda;
      this->_scl = _scl;
	  _i2c = new I2C(_sda, _scl);
    }

    bool connect() {
		// mbed supports 100k and 400k some device maybe 1000k
#ifdef TARGET_STM32L4
	  _i2c->frequency(1000000);
#else
	  _i2c->frequency(400000);
#endif
      return true;
    }

    void display(void) {
      const int x_offset = (128 - this->width()) / 2;
#ifdef OLEDDISPLAY_DOUBLE_BUFFER
        uint8_t minBoundY = UINT8_MAX;
        uint8_t maxBoundY = 0;

        uint8_t minBoundX = UINT8_MAX;
        uint8_t maxBoundX = 0;
        uint8_t x, y;

        // Calculate the Y bounding box of changes
        // and copy buffer[pos] to buffer_back[pos];
        for (y = 0; y < (this->height() / 8); y++) {
          for (x = 0; x < this->width(); x++) {
           uint16_t pos = x + y * this->width();
           if (buffer[pos] != buffer_back[pos]) {
             minBoundY = std::min(minBoundY, y);
             maxBoundY = std::max(maxBoundY, y);
             minBoundX = std::min(minBoundX, x);
             maxBoundX = std::max(maxBoundX, x);
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
        sendCommand(x_offset + minBoundX);	// column start address (0 = reset)
        sendCommand(x_offset + maxBoundX);	// column end address (127 = reset)

        sendCommand(PAGEADDR);
        sendCommand(minBoundY);				// page start address
        sendCommand(maxBoundY);				// page end address

        for (y = minBoundY; y <= maxBoundY; y++) {
			uint8_t *start = &buffer[(minBoundX + y * this->width())-1];
			uint8_t save = *start;
			
			*start = 0x40; // control
			_i2c->write(_address, (char *)start, (maxBoundX-minBoundX) + 1 + 1);
			*start = save;
		}
#else

        sendCommand(COLUMNADDR);
        sendCommand(x_offset);						// column start address (0 = reset)
        sendCommand(x_offset + (this->width() - 1));// column end address (127 = reset)

        sendCommand(PAGEADDR);
        sendCommand(0x0);							// page start address (0 = reset)

        if (geometry == GEOMETRY_128_64) {
          sendCommand(0x7);
        } else if (geometry == GEOMETRY_128_32) {
          sendCommand(0x3);
        }

		buffer[-1] = 0x40; // control
		_i2c->write(_address, (char *)&buffer[-1], displayBufferSize + 1);
#endif
    }

private:
	int getBufferOffset(void) {
		return 0;
	}

    inline void sendCommand(uint8_t command) __attribute__((always_inline)) {
		char _data[2];
	  	_data[0] = 0x80; // control
	  	_data[1] = command;
	  	_i2c->write(_address, _data, sizeof(_data));
    }

	uint8_t             _address;
	PinName             _sda;
	PinName             _scl;
	I2C *_i2c;
};

#endif

#endif
