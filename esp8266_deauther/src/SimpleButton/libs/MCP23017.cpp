#include "MCP23017.h"
namespace simplebutton {
    MCP23017::MCP23017(uint8_t address) {
        setup(address);
    }

    MCP23017::MCP23017(uint8_t address, TwoWire* wire) {
        setup(address, wire);
    }

    MCP23017::~MCP23017() {}

    void MCP23017::setup(uint8_t address) {
        setup(address, &Wire);
    }

    void MCP23017::setup(uint8_t address, TwoWire* wire) {
        this->address = address;
        this->wire    = wire;

        setIO();
        setPullups();
    }

    int MCP23017::read() {
        this->pinData = readRegister16(0x12); // 0x12 = GPIOA

        return this->pinData;
    }

    int MCP23017::read(uint8_t pin) {
        if (pin >= 16) {
            error = PCF_PIN_ERROR;
            return 0;
        }

        // make sure the pin is set to be an input
        if (getPinMode(pin) == OUTPUT) {
            bool pullup = getPinState(pin);
            setPinMode(pin, pullup ? INPUT_PULLUP : INPUT);
        }

        return (read() >> pin) & 0x1;
    }

    void MCP23017::write(int value) {
        // make sure all pins are set as outputs
        for (int i = 0; i < 16; i++) {
            bool output = (value >> i) & 0x1;
            if (output && (getPinMode(i) != OUTPUT)) setPinMode(i, OUTPUT);
        }

        this->pinData = value;

        writeRegister(0x12, value); // 0x12 = GPIOA
    }

    void MCP23017::write(uint8_t pin, bool value) {
        if (pin >= 16) {
            error = PCF_PIN_ERROR;
            return;
        }

        if (getPinState(pin) != value) toggle(pin);
    }

    void MCP23017::toggle() {
        pinData = ~pinData;
        write(pinData);
    }

    void MCP23017::toggle(uint8_t pin) {
        if (pin >= 16) {
            error = PCF_PIN_ERROR;
            return;
        }

        pinData ^= 1 << pin;
        write(pinData);
    }

    void MCP23017::setIO() {
        writeRegister(0x00, this->pinModes); // 0x00 = IODIRA register
    }

    void MCP23017::setPullups() {
        writeRegister(0x0C, this->pinPullups); // 0x0C = GPPUA register
    }

    void MCP23017::setPinMode(uint8_t pin, uint8_t mode) {
        if (pin >= 16) {
            error = PCF_PIN_ERROR;
            return;
        }

        if (getPinMode(pin) == mode) return;

        bool input  = (mode == INPUT || mode == INPUT_PULLUP);
        bool pullup = (mode == INPUT_PULLUP);

        bitWrite(pinModes, pin, input);
        bitWrite(pinPullups, pin, pullup);

        setIO();
        setPullups();
    }

    uint8_t MCP23017::getPinMode(uint8_t pin) {
        if (pin >= 16) {
            error = PCF_PIN_ERROR;
            return 0;
        }

        bool input  = (this->pinModes >> pin) & 0x1;
        bool pullup = (this->pinPullups >> pin) & 0x1;

        if (!input) return OUTPUT;

        if (pullup) return INPUT_PULLUP;

        return INPUT;
    }

    bool MCP23017::getPinState(uint8_t pin) {
        if (pin >= 16) {
            error = PCF_PIN_ERROR;
            return false;
        }

        return (pinData >> pin) & 0x1;
    }

    uint8_t MCP23017::readRegister8(uint8_t address) {
        wire->beginTransmission(this->address);
        wire->write(address);
        error = wire->endTransmission();

        wire->requestFrom(this->address, (uint8_t)1);

        if (wire->available() == 1) {
            return wire->read();
        } else {
            error = PCF_I2C_ERROR;
            return 0;
        }
    }

    uint16_t MCP23017::readRegister16(uint8_t address) {
        wire->beginTransmission(this->address);
        wire->write(address);
        error = wire->endTransmission();

        wire->requestFrom(this->address, (uint8_t)2);

        if (wire->available() == 2) {
            uint16_t dataA = wire->read();
            uint16_t dataB = wire->read();

            return (dataB << 8) | dataA;
        } else {
            error = PCF_I2C_ERROR;
            return 0;
        }
    }

    void MCP23017::writeRegister(uint8_t address, uint16_t value) {
        wire->beginTransmission(this->address);
        wire->write(address);
        wire->write(value & 0xFF);
        wire->write(value >> 8);
        error = wire->endTransmission();
    }
}