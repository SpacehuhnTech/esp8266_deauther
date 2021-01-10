#ifndef SimpleButton_MCP23017_h
#define SimpleButton_MCP23017_h

#include "GPIOExpander.h"

namespace simplebutton {
    class MCP23017 : public GPIOExpander {
        public:
            MCP23017(uint8_t address);
            MCP23017(uint8_t address, TwoWire* wire);

            ~MCP23017();

            void setup(uint8_t address);
            void setup(uint8_t address, TwoWire* wire);

            int read();
            int read(uint8_t pin);

            void write(int value);
            void write(uint8_t pin, bool value);

            void toggle();
            void toggle(uint8_t pin);

        private:
            uint16_t pinData    = 0x0000;
            uint16_t pinModes   = 0x0000;
            uint16_t pinPullups = 0x0000;

            void setIO();
            void setPullups();

            void setPinMode(uint8_t pin, uint8_t mode);
            uint8_t getPinMode(uint8_t pin);

            bool getPinState(uint8_t pin);

            uint8_t readRegister8(uint8_t address);
            uint16_t readRegister16(uint8_t address);

            void writeRegister(uint8_t address, uint16_t value);
    };
}

#endif // ifndef SimpleButton_MCP23017_h