#ifndef StatusLED_H
#define StatusLED_H

#include "Arduino.h"

class StatusLED {
    public:
        virtual ~StatusLED() = default;

        virtual void setup() = 0;

        virtual void setColor(uint8_t r, uint8_t g, uint8_t b) = 0;
        virtual void setBrightness(uint8_t brightness) = 0;
};

#endif // ifndef StatusLED_H