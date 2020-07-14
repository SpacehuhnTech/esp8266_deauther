/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h>

namespace wifi {
    void begin();

    String getMode();
    void printStatus();
    
    void startNewAP(String path, String ssid, String password, uint8_t ch, bool hidden, bool captivePortal);
    //void startAP(String path);
    void startAP();

    void stopAP();
    void resumeAP();
    
    void update();
}