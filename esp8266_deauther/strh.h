/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

namespace strh {
    String whitespace(int len);
    String left(int len, String str);
    String right(int len, String str);
    String center(int len, String str);
    String mac(const uint8_t* b);
    String escape(String str);
    String time(unsigned long time);
    String channels(uint16_t reg);
}