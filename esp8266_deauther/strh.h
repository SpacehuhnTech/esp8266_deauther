/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

namespace strh {
    void hide_mac(bool mode);
    String whitespace(int len);
    String left(int len, String str);
    String right(int len, String str);
    String center(int len, String str);
    String mac(const uint8_t* b, unsigned int len = 6);
    String escape(String str);
    String time(unsigned long time);
    String channels(uint16_t reg);
    String boolean(bool value);
}