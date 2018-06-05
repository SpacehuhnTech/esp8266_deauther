#include "AnalogRGBLED.h"

AnalogRGBLED::AnalogRGBLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, uint8_t brightness, bool anode) {
    AnalogRGBLED::anode = anode;
    AnalogRGBLED::rPin  = rPin;
    AnalogRGBLED::gPin  = gPin;
    AnalogRGBLED::bPin  = bPin;
    setBrightness(brightness);
}

AnalogRGBLED::~AnalogRGBLED() {}

void AnalogRGBLED::setup() {
    analogWriteRange(0xff);

    if (rPin < 255) pinMode(rPin, OUTPUT);

    if (gPin < 255) pinMode(gPin, OUTPUT);

    if (bPin < 255) pinMode(bPin, OUTPUT);
}

void AnalogRGBLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if ((r > 0) && (brightness < 100)) r = r * brightness / 100;

    if ((g > 0) && (brightness < 100)) g = g * brightness / 100;

    if ((b > 0) && (brightness < 100)) b = b * brightness / 100;

    if (anode) {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
    }

    analogWrite(rPin, r);
    analogWrite(gPin, g);
    analogWrite(bPin, b);
}

void AnalogRGBLED::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    AnalogRGBLED::brightness = brightness;
}