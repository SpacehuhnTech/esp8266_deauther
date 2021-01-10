#include "RotaryEncoderI2C.h"
namespace simplebutton {
    RotaryEncoderI2C::RotaryEncoderI2C() {
        setup(0x30);
    }

    RotaryEncoderI2C::RotaryEncoderI2C(uint8_t i2cAddress) {
        setup(i2cAddress);
    }

    RotaryEncoderI2C::RotaryEncoderI2C(uint8_t i2cAddress, TwoWire* wire) {
        setup(i2cAddress, wire);
    }

    RotaryEncoderI2C::~RotaryEncoderI2C() {}

    void RotaryEncoderI2C::setup(uint8_t i2cAddress) {
        setup(i2cAddress, &Wire);
    }

    void RotaryEncoderI2C::setup(uint8_t i2cAddress, TwoWire* wire) {
        this->i2cAddress = i2cAddress;
        this->wire       = wire;

        this->clockwise     = new Button();
        this->anticlockwise = new Button();
        this->button        = new Button();

        setMin(-128);
        setMax(127);

        begin();
    }

    bool RotaryEncoderI2C::interrupt() {
        if (interruptEnable) return digitalRead(interruptPin) == LOW;

        return true;
    }

    void RotaryEncoderI2C::enableInterrupt(uint8_t pin, bool pullup) {
        interruptPin    = pin;
        interruptEnable = true;
        interruptPullup = pullup;
        pinMode(pin, INPUT);
    }

    bool RotaryEncoderI2C::update() {
        if (interrupt()) {
            readStatus();

            if (clicked()) button->click();

            if (incremented()) clockwise->click();

            if (decremented()) anticlockwise->click();

            return true;
        }

        return false;
    }

    void RotaryEncoderI2C::begin() {
        uint8_t config = 0x00;

        if (interruptEnable) config = config | 0x01;
        if (ledEnabled) config = config | 0x02;
        if (loop) config = config | 0x04;
        if (inverted) config = config | 0x08;
        if (!interruptPullup) config = config | 0x10;
        if (encoding) config = config | 0x20;

        setConfig(config);
    }

    void RotaryEncoderI2C::reset() {
        button->reset();
        clockwise->reset();
        anticlockwise->reset();

        setConfig(0x80);
        update();
    }

    bool RotaryEncoderI2C::connected() {
        return error == 0;
    }

    String RotaryEncoderI2C::getError() {
        String msg;

        switch (error) {
        case 0:
            msg += "OK";
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
            msg += String(F("I2C error"));
            break;
        }

        return msg;
    }

    void RotaryEncoderI2C::setConfig(uint8_t config) {
        write(0x00, config);
    }

    void RotaryEncoderI2C::enableLed(bool led) {
        ledEnabled = led;
    }

    void RotaryEncoderI2C::enableLoop(bool loop) {
        this->loop = loop;
    }

    void RotaryEncoderI2C::setEncoding(uint8_t encoding) {
        if (encoding == 1) this->encoding = false;
        else if (encoding == 2) this->encoding = true;
    }

    void RotaryEncoderI2C::setInverted(bool inverted) {
        this->inverted = inverted;
    }

    void RotaryEncoderI2C::setPos(int32_t value) {
        write(0x02, value);
    }

    void RotaryEncoderI2C::setMin(int32_t value) {
        write(0x0A, value);
    }

    void RotaryEncoderI2C::setMax(int32_t value) {
        write(0x06, value);
    }

    void RotaryEncoderI2C::setLed(uint8_t valueA, uint8_t valueB) {
        setLedA(valueA);
        setLedB(valueB);
    }

    void RotaryEncoderI2C::setLedA(uint8_t value) {
        if (ledEnabled) write(0x0E, value);
    }

    void RotaryEncoderI2C::setLedB(uint8_t value) {
        if (ledEnabled) write(0x0F, value);
    }

    int32_t RotaryEncoderI2C::getPos() {
        return read32(0x02);
    }

    uint8_t RotaryEncoderI2C::readStatus() {
        status = read(0x01);
        return status;
    }

    uint8_t RotaryEncoderI2C::readLedA() {
        return read(0x0E);
    }

    uint8_t RotaryEncoderI2C::readLedB() {
        return read(0x0F);
    }

    int32_t RotaryEncoderI2C::readMax() {
        return read32(0x06);
    }

    int32_t RotaryEncoderI2C::readMin() {
        return read32(0x0A);
    }

    bool RotaryEncoderI2C::clicked() {
        return status & 0x01;
    }

    bool RotaryEncoderI2C::incremented() {
        return status & 0x02;
    }

    bool RotaryEncoderI2C::decremented() {
        return status & 0x04;
    }

    bool RotaryEncoderI2C::minVal() {
        return status & 0x10;
    }

    bool RotaryEncoderI2C::maxVal() {
        return status & 0x08;
    }

    void RotaryEncoderI2C::write(uint8_t address, uint8_t value) {
        wire->beginTransmission(i2cAddress);

        wire->write(address);
        wire->write(value);

        error = wire->endTransmission();
    }

    void RotaryEncoderI2C::write(uint8_t address, int32_t value) {
        wire->beginTransmission(i2cAddress);

        wire->write(address);
        wire->write(((uint32_t)value >> 24) & 0xFF);
        wire->write(((uint32_t)value >> 16) & 0xFF);
        wire->write(((uint32_t)value >> 8) & 0xFF);
        wire->write((uint32_t)value & 0xFF);

        error = wire->endTransmission();
    }

    uint8_t RotaryEncoderI2C::read(uint8_t address) {
        uint8_t data = 0xFF;

        // ask for some sweet data

        wire->beginTransmission(i2cAddress);
        wire->write(address);
        error = wire->endTransmission();

        // read out the sweet data
        wire->requestFrom(i2cAddress, (uint8_t)1);

        if (wire->available() == 1) {
            data = wire->read();
        } else {
            error = ROTARY_ENCODER_I2C_ERROR;
        }

        return data;
    }

    int32_t RotaryEncoderI2C::read32(uint8_t address) {
        uint32_t data = 0xFFFFFFFF;

        // ask for some sweet data
        wire->beginTransmission(i2cAddress);
        wire->write(address);
        error = wire->endTransmission();

        // read out the sweet data
        wire->requestFrom(i2cAddress, (uint8_t)4);

        if (wire->available() == 4) {
            data = wire->read();
            data = (data << 8) | wire->read();
            data = (data << 8) | wire->read();
            data = (data << 8) | wire->read();
        } else {
            error = ROTARY_ENCODER_I2C_ERROR;
        }

        return (int32_t)data;
    }
}