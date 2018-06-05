#ifndef LEDController_h
#define LEDController_h

#include "Arduino.h"
extern "C" {
  #include "user_interface.h"
}
#include "language.h"
#include "A_config.h"
#include "Settings.h"
#include "Attack.h"
#include "Scan.h"

#include <Adafruit_NeoPixel.h>

extern Settings settings;
extern Attack   attack;
extern Scan     scan;
extern Stations stations;

class LEDController {
    public:
        enum LED_MODE { OFF = 0, SCAN = 1, ATTACK = 2, DEAUTH = 3, IDLE = 4 };

        LEDController();
        ~LEDController();

        void setup();
        void update();

        void setMode(uint8_t mode, bool force);

        void setColor(uint8_t r, uint8_t g, uint8_t b, bool output);
        void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output);

        void tempEnable();
        void tempDisable();
        bool getTempEnabled();

    private:
        class StatusLED {
            public:
                virtual ~StatusLED() = default;

                virtual void setup() = 0;

                virtual void setColor(uint8_t r, uint8_t g, uint8_t b) = 0;
                virtual void setBrightness(uint8_t brightness) = 0;
        };

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

        bool tempEnabled = true;
        uint8_t mode     = LED_MODE::OFF;
        StatusLED* led   = NULL;
};

#endif // ifndef LEDController_h