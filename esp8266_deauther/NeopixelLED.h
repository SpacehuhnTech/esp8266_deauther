#ifndef NeopixelLED_H
#define NeopixelLED_H

#include "StatusLED.h"
#include <Adafruit_NeoPixel.h>

class NeopixelLED : public StatusLED {
    public:
        NeopixelLED(int num, uint8_t dataPin, uint8_t brightness);
        ~NeopixelLED();

        void setup();
        void setColor(uint8_t r, uint8_t g, uint8_t b);
        void setBrightness(uint8_t brightness);
        void setMode(uint8_t mode, bool force);

    private:
        Adafruit_NeoPixel* strip;
};

#endif // ifndef NeopixelLED_H