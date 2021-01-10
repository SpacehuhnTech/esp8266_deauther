#ifndef SimpleButton_PCF8574_h
#define SimpleButton_PCF8574_h

#include "GPIOExpander.h"

namespace simplebutton {
    class PCF8574 : public GPIOExpander {
        public:
            PCF8574(uint8_t address);
            PCF8574(uint8_t address, TwoWire* wire);
            ~PCF8574();

            int read();
            int read(uint8_t pin);

            void write(int value);
            void write(uint8_t pin, bool value);

            void toggle();
            void toggle(uint8_t pin);

        private:
            uint8_t data;
            uint8_t pinModeMask;
    };
}

#endif // ifndef SimpleButton_PCF8574_h