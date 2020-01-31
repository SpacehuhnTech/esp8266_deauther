/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "StringList.h"
#include "packetinjector.h"

namespace attack {
    class TargetList;
    class Target;

    typedef struct target_t target_t;

    class TargetList {
        private:
            target_t* list_begin = NULL;
            target_t* list_end   = NULL;
            int list_size        = 0;

            target_t* h = NULL;

        public:
            ~TargetList();

            void push(const uint8_t* from, const uint8_t* to, const uint8_t ch);

            Target get(int i);

            void begin();
            Target iterate();

            bool available();
            int size();
    };

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

    void beacon(StringList& ssid_list, uint8_t* from, uint8_t* to, Encryption enc, uint8_t ch, unsigned long timeout);
    void deauth(TargetList& targets, bool deauth, bool disassoc, unsigned long rate, unsigned long timeout, unsigned long pkts);
}