#include "NeopixelLED.h"

NeopixelLED::NeopixelLED(int num, uint8_t dataPin, uint8_t brightness) {
    strip = new Adafruit_NeoPixel(num, dataPin, NEO_GRB + NEO_KHZ800);
    setBrightness(brightness);
}

NeopixelLED::~NeopixelLED() {
    delete strip;
}

void NeopixelLED::setup() {
    strip->begin();
    strip->show();
}

void NeopixelLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    int num = strip->numPixels();

    for (uint16_t i = 0; i < num; i++) strip->setPixelColor(i, strip->Color(r, g, b));
    strip->show();
}

void NeopixelLED::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    strip->setBrightness(brightness);
}