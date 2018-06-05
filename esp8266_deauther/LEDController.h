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

#include "StatusLED.h"
#include "DigitalLED.h"
#include "NeopixelLED.h"
#include "AnalogRGBLED.h"

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
        bool tempEnabled = true;
        uint8_t mode     = LED_MODE::OFF;
        StatusLED* led   = NULL;
};

#endif // ifndef LEDController_h