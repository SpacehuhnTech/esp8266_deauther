#include "DigitalLED.h"

DigitalLED::DigitalLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, bool anode) {
    DigitalLED::anode = anode;
    DigitalLED::rPin  = rPin;
    DigitalLED::gPin  = gPin;
    DigitalLED::bPin  = bPin;
}

DigitalLED::~DigitalLED() {}

void DigitalLED::setup() {
    if (rPin < 255) pinMode(rPin, OUTPUT);

    if (gPin < 255) pinMode(gPin, OUTPUT);

    if (bPin < 255) pinMode(bPin, OUTPUT);
}

void DigitalLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if (anode) {
        if (rPin < 255) digitalWrite(rPin, r > 0);

        if (gPin < 255) digitalWrite(gPin, g > 0);

        if (bPin < 255) digitalWrite(bPin, b > 0);
    } else {
        if (rPin < 255) digitalWrite(rPin, r == 0);

        if (gPin < 255) digitalWrite(gPin, g == 0);

        if (bPin < 255) digitalWrite(bPin, b == 0);
    }
}

void DigitalLED::setBrightness(uint8_t brightness) {}