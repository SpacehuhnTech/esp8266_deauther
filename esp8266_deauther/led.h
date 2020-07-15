/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

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