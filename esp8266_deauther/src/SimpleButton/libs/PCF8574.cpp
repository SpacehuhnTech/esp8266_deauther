#include "PCF8574.h"

namespace simplebutton {
    PCF8574::PCF8574(uint8_t address) {
        setup(address);
    }

    PCF8574::PCF8574(uint8_t address, TwoWire* wire) {
        setup(address, wire);
    }

    PCF8574::~PCF8574() {}

    int PCF8574::read() {
        wire->requestFrom(address, (uint8_t)1);

        data = 0;

        if (wire->available() >= 1) {
            data = wire->read();
        } else {
            error = PCF_I2C_ERROR;
        }

        return data;
    }

    int PCF8574::read(uint8_t pin) {
        if (pin < 8) {
            data = read();
            return (data & (1 << pin)) > 0;
        } else {
            error = PCF_PIN_ERROR;
            return -1;
        }
    }

    void PCF8574::write(int value) {
        wire->beginTransmission(address);

        pinModeMask &= 0xff00;
        pinModeMask |= value;
        data         = pinModeMask;

        wire->write(data);

        error = wire->endTransmission();
    }

    void PCF8574::write(uint8_t pin, bool value) {
        if (pin >= 8) {
            error = PCF_PIN_ERROR;
            return;
        }

        if (value) pinModeMask |= value << pin;
        else pinModeMask &= ~(1 << pin);

        write(pinModeMask);
    }

    void PCF8574::toggle() {
        pinModeMask = ~pinModeMask;
        write(pinModeMask);
    }

    void PCF8574::toggle(uint8_t pin) {
        if (pin < 8) {
            pinModeMask ^= 1 << pin;

            write(pinModeMask);
        } else {
            error = PCF_PIN_ERROR;
        }
    }
}