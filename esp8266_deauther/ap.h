/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h>

namespace ap {
    void start(String& ssid, String& pswd, bool hidden, uint8_t ch, uint8_t* bssid);
    void stop();
    void pause();
    void resume();
    bool paused();
    void update();
}