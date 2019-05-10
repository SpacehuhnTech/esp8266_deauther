#include "LED.h"

// Strings used in printColor and tempDisable
#include "language.h"

// For Update()
#include "Settings.h"
#include "Attack.h"
#include "Scan.h"

extern Settings settings;
extern Attack   attack;
extern Scan     scan;

void LED::update() {
    if (!tempEnabled) return;

    if (!settings.getLedEnabled() && tempEnabled) {
        tempDisable();
    }

    if (scan.isScanning() && (scan.deauths < settings.getMinDeauths())) {
        setMode(SCAN);
    } else if (attack.isRunning() || (scan.deauths >= settings.getMinDeauths())) {
        setMode(ATTACK);
    } else {
        setMode(IDLE);
    }
}

void LED::printColor(uint8_t r, uint8_t g, uint8_t b) {
    char s[30];

    sprintf_P(s, L_OUTPUT, r, g, b);
    prnt(String(s));
}

void LED::setMode(LED_MODE mode, bool force) {
    if ((mode != this->mode) || force) {
        this->mode = mode;

        switch (mode) {
            case OFF:
                setColor(0, 0, 0);
                break;

            case SCAN:
                setColor(0, 0, 255);
                break;

            case ATTACK:
                setColor(255, 0, 0);
                break;

            case IDLE:
                setColor(0, 255, 0);
                break;
        }
    }
}

void LED::setBrightness(uint8_t brightness) {
    this->brightness = brightness % 100;
}

void LED::tempEnable() {
    tempEnabled = true;
    prntln(L_ENABLED);
}

void LED::tempDisable() {
    tempEnabled = false;
    prntln(L_DISABLED);
}

bool LED::getTempEnabled() {
    return tempEnabled;
}