/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>  // uint8_t
#include <stdbool.h> // bool

namespace mac {
    const static uint8_t BROADCAST[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    typedef struct item_t {
        uint8_t data[6];
        item_t* next;
    } item_t;

    typedef struct list_t {
        item_t* begin;
        item_t* end;
        int     size;
    } list_t;

    bool valid(const uint8_t* mac);
    bool multicast(const uint8_t* mac);
    bool equals(const uint8_t* macA, const uint8_t* macB);

    void fromStr(const char* str, uint8_t* mac);

    list_t createList();
    void push(list_t* list, const uint8_t* mac);
    void clear(list_t* list);
}