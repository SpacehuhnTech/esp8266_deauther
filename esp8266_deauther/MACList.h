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
        typedef struct mac_t {
            uint8_t addr[6];
            mac_t * next;
        } mac_t;

        mac_t* list_begin = NULL;
        mac_t* list_end   = NULL;

        int list_size     = 0;
        int list_max_size = 0;

        mac_t* list_h = NULL;
        int list_pos  = 0;

        int compare(const mac_t* a, const uint8_t* b) const;

    public:
        MACList(int max = 0);
        ~MACList();

        void moveFrom(MACList& t);

        bool push(const uint8_t* addr);

        uint8_t* get(int i);

        void begin();
        uint8_t* iterate();

        bool available() const;
        int size() const;
        bool full() const;
        bool contains(const uint8_t* mac) const;

        void clear();
};