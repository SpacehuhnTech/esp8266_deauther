/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

// #include <stdint.h>  // uint8_t
#include <Arduino.h> // String

namespace alias {
    void clear();
    void load();
    void save();

    bool add(const uint8_t* mac, const String& name);
    String get(const uint8_t* mac);
    bool resolve(const String& name, uint8_t* buffer);
    void print();
}