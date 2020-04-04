/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

typedef void (* search_cb_f)(const uint8_t* prefix, const char* name);

namespace vendor {
    void getRandomMac(uint8_t* mac);
    String getName(const uint8_t* mac);
    void getMAC(String name, bool substring, search_cb_f cb);
};