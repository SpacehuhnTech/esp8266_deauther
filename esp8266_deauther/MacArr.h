/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// #include <stdint.h>
// #include <stddef.h>
#include <Arduino.h> // String
#include "config.h"

class MacArr {
    private:
        typedef struct mac_t {
            uint8_t mac[6];
        } mac_t;

        typedef struct mac_list_t {
            mac_t      * data;
            unsigned int max;
            unsigned int size;
            unsigned int h;
        } mac_list_t;

        mac_list_t list { nullptr, 0, 0, 0 };

        int search(const uint8_t* mac) const;
        int bin_search(const uint8_t* mac, int low_end, int up_end) const;

    public:
        MacArr(int size = 0);
        MacArr(const MacArr& ml);
        MacArr(MacArr&& ml);
        MacArr(const String& input, String delimiter = ",");
        ~MacArr();

        MacArr& operator=(const MacArr& ml);
        MacArr& operator=(MacArr&& ml);

        MacArr& operator+=(const MacArr& ml);

        void clear();

        void parse(const String& input, String delimiter = ",");

        bool add(const uint8_t* mac);

        bool contains(const uint8_t* mac) const;

        void begin();
        const uint8_t* iterate();

        bool available() const;
        int size() const;
        bool full() const;
        bool empty() const;
};