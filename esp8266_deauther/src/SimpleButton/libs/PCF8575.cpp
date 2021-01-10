#include "PCF8575.h"

namespace simplebutton {
    PCF8575::PCF8575(uint8_t address) {
        setup(address);
    }

    PCF8575::PCF8575(uint8_t address, TwoWire* wire) {
        setup(address, wire);
    }

    PCF8575::~PCF8575() {}

    int PCF8575::read() {
        wire->requestFrom(address, (uint8_t)2);

        data = 0;

        if (wire->available() >= 2) {
            data  = wire->read();
            data |= wire->read() << 8;
        }

        return data;
    }

    int PCF8575::read(uint8_t pin) {
        data = read();

        return (data & (1 << pin)) > 0;
    }

    void PCF8575::write(int value) {
        wire->beginTransmission(address);

        pinModeMask = value;
        data        = pinModeMask;

        wire->write((uint8_t)data);
        wire->write((uint8_t)(data >> 8));

        wire->endTransmission();
    }

    void PCF8575::write(uint8_t pin, bool value) {
        if (value) pinModeMask |= value << pin;
        else pinModeMask &= ~(1 << pin);

        write(pinModeMask);
    }

    void PCF8575::toggle() {
        pinModeMask = ~pinModeMask;
        write(pinModeMask);
    }

    void PCF8575::toggle(uint8_t pin) {
        pinModeMask ^= 1 << pin;

        write(pinModeMask);
    }
}