#ifndef DigitalLED_H
#define DigitalLED_H

#include "StatusLED.h"

class DigitalLED : public StatusLED {
    public:
        DigitalLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, bool anode);
        ~DigitalLED();

        void setup();
        void setColor(uint8_t r, uint8_t g, uint8_t b);
        void setBrightness(uint8_t brightness);
        void setMode(uint8_t mode, bool force);

    private:
        bool anode   = true;
        uint8_t rPin = 255;
        uint8_t gPin = 255;
        uint8_t bPin = 255;
};

#endif // ifndef DigitalLED_H