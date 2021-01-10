#include "GPIOExpander.h"

namespace simplebutton {
    void GPIOExpander::setup(uint8_t address) {
        this->wire    = &Wire;
        this->address = address;
        write(0);
    }

    void GPIOExpander::setup(uint8_t address, TwoWire* wire) {
        this->wire    = wire;
        this->address = address;
        write(0);
    }

    bool GPIOExpander::connected() {
        return error == 0;
    }

    String GPIOExpander::getError() {
        String msg;

        switch (error) {
        case 0:
            msg += String(F("OK"));
            break;

        case 1:
            msg += String(F("Data too long to fit in transmit buffer"));
            break;

        case 2:
            msg += String(F("Received NACK on transmit of address"));
            break;

        case 3:
            msg += String(F("Received NACK on transmit of data"));

        case 4:
            msg += String(F("Unknown transmission error"));
            break;

        case 5:
            msg += String(F("Pin error"));
            break;

        case 6:
            msg += String(F("I2C error"));
            break;
        }

        return msg;
    }
}