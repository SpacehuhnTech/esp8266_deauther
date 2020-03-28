/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

namespace cli {
    void begin();
    void parse(const char* input);

    bool available();
    bool read(String& destination, const char* _default = nullptr);
    bool read_exit();

    void update();
}