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

    int search(const uint8_t* mac);
    int search(const String& name);

    bool add(const uint8_t* mac, const String& name);

    String get(const uint8_t* mac);
    bool resolve(const String& name, uint8_t* buffer, unsigned int len = 6);

    String getName(int id);

    bool remove(int id);
    bool remove(const uint8_t* mac);
    bool remove(const String& name);

    void print();
}