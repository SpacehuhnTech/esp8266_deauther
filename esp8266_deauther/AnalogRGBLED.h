#ifndef AnalogRGBLED_H
#define AnalogRGBLED_H

#include "StatusLED.h"

class AnalogRGBLED : public StatusLED {
    public:
        AnalogRGBLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, uint8_t brightness, bool anode);
        ~AnalogRGBLED();

        void setup();
        void setColor(uint8_t r, uint8_t g, uint8_t b);
        void setBrightness(uint8_t brightness);
        void setMode(uint8_t mode, bool force);

    private:
        bool anode         = true;
        uint8_t rPin       = 255;
        uint8_t gPin       = 255;
        uint8_t bPin       = 255;
        uint8_t brightness = 0;
};
#endif // ifndef AnalogRGBLED_H