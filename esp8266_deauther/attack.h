/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "StringList.h"

#define ENCRYPTION_OPEN 0
// #define ENCRYPTION_WEP 1
// #define ENCRYPTION_WPA 2
#define ENCRYPTION_WPA2 3
// #define ENCRYPTION_WPA_AUTO 4
// #define ENCRYPTION_WPA3 5

namespace attack {
    class TargetList;
    class Target;

    typedef struct target_t target_t;

    class Target {
        private:
            target_t* ptr;

        public:
            Target(target_t* ptr);

            uint8_t* from() const;
            uint8_t* to() const;
            uint8_t ch() const;

            bool operator==(const Target& t) const;
    };

    class TargetList {
        private:
            target_t* list_begin = NULL;
            target_t* list_end   = NULL;

            int list_size = 0;

            target_t* h = NULL;

        public:
            ~TargetList();

            void move(TargetList& t);

            void push(const uint8_t* from, const uint8_t* to, const uint8_t ch);

            Target get(int i);

            void begin();
            Target iterate();

            bool available();
            int size();
    };

    void beacon(StringList& ssid_list, uint8_t* from, uint8_t* to, int enc, uint8_t ch, unsigned long timeout);
    void deauth(TargetList& targets, bool deauth, bool disassoc, unsigned long rate, unsigned long timeout, unsigned long pkts);
}