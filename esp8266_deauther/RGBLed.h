#ifndef RGBLed_h
#define RGBLed_h

#include "Arduino.h"
#include "Settings.h"
#include "Attack.h"
#include "Scan.h"
#include "language.h"
#include "A_config.h"

#define LED_MODE_OFF 0
#define LED_MODE_SCAN 1
#define LED_MODE_ATTACK 2
#define LED_MODE_DEAUTH 3
#define LED_MODE_IDLE 4

extern Settings settings;
extern Attack attack;
extern Scan scan;
extern Stations stations;

class RGBLed {
  public:
    RGBLed();
    void setup();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
    void setMode(uint8_t mode, bool force);
    void update();
    void tempEnable();
    void tempDisable();
    bool getTempEnabled();
  private:
    uint8_t mode;
    bool tempEnabled = true;
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output);
};

#endif



