/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

namespace vendor {
    void randomize(uint8_t* mac);
    String find(const uint8_t* mac);
};