/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "config.h"

typedef struct mac_t mac_t;

class MACList {
    private:
        mac_t* list_begin = NULL;
        mac_t* list_end   = NULL;

        int list_size     = 0;
        int list_max_size = 0;

        mac_t* h = NULL;

    public:
        MACList(int max = MAX_TARGETS);
        ~MACList();

        void moveFrom(MACList& t);

        bool push(const uint8_t* addr);

        uint8_t* get(int i);

        void begin();
        uint8_t* iterate();

        bool available() const;
        int size() const;
        bool full() const;

        void clear();
};