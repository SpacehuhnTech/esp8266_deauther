/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "config.h"


class MAC {
    private:
        uint8_t addr[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        char* name      = NULL;
        MAC* next       = NULL;

    public:
        MAC(const uint8_t* addr, const char* name);
        ~MAC();

        const char* getName() const;
        const uint8_t* getAddr() const;

        MAC* getNext();
        void setNext(MAC* next);
};

class MACList {
    private:
        MAC* list_begin = NULL;
        MAC* list_end   = NULL;

        int list_size     = 0;
        int list_max_size = 0;

        MAC* list_h  = NULL;
        int list_pos = 0;

        int compare(const MAC* a, const uint8_t* b) const;
        int compare(const MAC* a, const MAC* b) const;

    public:
        MACList(int max = 0);
        ~MACList();

        bool push(const uint8_t* addr, const char* name);
        MAC* search(const uint8_t* mac);
        void clear();

        MAC* get(int i);

        void begin();
        MAC* iterate();

        bool available() const;
        int size() const;
        bool full() const;
};