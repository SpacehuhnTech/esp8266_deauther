/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#pragma once

enum LED_MODE {
    OFF,
    SCAN,
    ATTACK,
    IDLE
};

namespace led {
    void setup();
    void update();
    void setMode(LED_MODE new_mode, bool force = false);
}