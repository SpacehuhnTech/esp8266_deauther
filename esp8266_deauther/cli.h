/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

namespace cli {
    void begin();
    void parse(const char* input);

    bool available();
    String read();
    String read_and_wait();
    bool read_exit();

    void update();
}