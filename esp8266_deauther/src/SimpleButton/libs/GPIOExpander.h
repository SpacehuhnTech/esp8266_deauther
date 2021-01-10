#ifndef SimpleButton_GPIOExpander_h
#define SimpleButton_GPIOExpander_h

#include "Arduino.h"
#include <Wire.h>

#define PCF_PIN_ERROR 5
#define PCF_I2C_ERROR 6

namespace simplebutton {
    class GPIOExpander {
        public:
            virtual ~GPIOExpander() = default;

            virtual void setup(uint8_t address);
            virtual void setup(uint8_t address, TwoWire* wire);

            virtual int read() = 0;
            virtual int read(uint8_t pin) = 0;

            virtual void write(int value) = 0;
            virtual void write(uint8_t pin, bool value) = 0;

            virtual void toggle() = 0;
            virtual void toggle(uint8_t pin) = 0;

            virtual bool connected();
            virtual String getError();

        protected:
            uint8_t error = 0;

            TwoWire* wire;
            uint8_t address;
    };
}
#endif // ifndef SimpleButton_GPIOExpander_h