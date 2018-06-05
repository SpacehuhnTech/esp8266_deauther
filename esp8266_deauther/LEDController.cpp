#include "LEDController.h"

LEDController::LEDController() {}

LEDController::~LEDController() {
    if (led) delete led;
}

void LEDController::setup() {
  #ifdef DIGITAL_LED
    led = new DigitalLED(LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_ANODE);
    led->setup();
    return;

  #endif // ifdef DIGITAL_LED

  #ifdef RGB_LED
    led = new AnalogRGBLED(LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_MODE_BRIGHTNESS, LED_ANODE);
    led->setup();
    return;

  #endif // ifdef RGB_LED

  #ifdef NEOPIXEL_LED
    led = new NeopixelLED(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, LED_MODE_BRIGHTNESS);
    led->setup();
    return;

  #endif // ifdef NEOPIXEL_LED

    prntln(L_NOT_CONFIGURED);
}

void LEDController::update() {
    if (!tempEnabled || !led) return;

    if (!settings.getLedEnabled() && tempEnabled) tempDisable();

    if (scan.isScanning() && (scan.deauths < settings.getMinDeauths())) setMode(LED_MODE::SCAN, false);
    else if (scan.deauths >= settings.getMinDeauths()) setMode(LED_MODE::DEAUTH, false);
    else if (attack.isRunning()) setMode(LED_MODE::ATTACK, false);
    else setMode(LED_MODE::IDLE, false);
}

void LEDController::setMode(uint8_t mode, bool force) {
    if (!led) return;

    if ((mode != LEDController::mode) || force) {
        LEDController::mode = mode;

        switch (mode) {
        case LED_MODE::OFF:
            led->setColor(0, 0, 0);
            break;

        case LED_MODE::SCAN:
            led->setColor(0, 0, 255);
            break;

        case LED_MODE::ATTACK:
            led->setColor(255, 255, 0);
            break;

        case LED_MODE::DEAUTH:
            led->setColor(255, 0, 0);
            break;

        case LED_MODE::IDLE:
            led->setColor(0, 255, 0);
            break;
        }
    }
}

void LEDController::setColor(uint8_t r, uint8_t g, uint8_t b, bool output) {
    // debug output
    if (output) {
        char s[30];
        sprintf_P(s, L_OUTPUT, r, g, b);
        prnt(String(s));
    }

    led->setColor(r, g, b);
}

void LEDController::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output) {
    led->setBrightness(brightness);
    setColor(r, g, b, output);
}

void LEDController::tempEnable() {
    tempEnabled = true;
    prntln(L_ENABLED);
}

void LEDController::tempDisable() {
    tempEnabled = false;
    prntln(L_DISABLED);
}

bool LEDController::getTempEnabled() {
    return tempEnabled;
}