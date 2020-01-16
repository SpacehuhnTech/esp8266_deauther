/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "mac.h"

#include <stdlib.h> // malloc
#include <string.h> // memcpy

namespace mac {
    // ===== PRIVATE ====== //
    unsigned int hex_to_int(const char* str, size_t len) {
        unsigned int res = 0;

        for (size_t i = 0; i < len; ++i) {
            uint8_t b = str[i];

            if ((b >= '0') && (b <= '9')) b = b - '0';
            else if ((b >= 'a') && (b <= 'f')) b = b - 'a' + 10;
            else if ((b >= 'A') && (b <= 'F')) b = b - 'A' + 10;

            res = (res << 4) | (b & 0xF);
        }

        return res;
    }

    // ===== PUBLIC ===== //
    bool valid(const uint8_t* mac) {
        for (uint8_t i = 0; i < 6; ++i) {
            if (mac[i] != 0x00) return true;
        }

        return false;
    }

    bool multicast(const uint8_t* mac) {
        return (mac[0] & 0x01) == 1;
    }

    bool equals(const uint8_t* macA, const uint8_t* macB) {
        for (uint8_t i = 0; i < 6; ++i) {
            if (macA[i] != macB[i]) return false;
        }

        return true;
    }

    void fromStr(const char* str, uint8_t* mac) {
        if (strlen(str) != 17) return;

        for (int i = 0; i<6; ++i) {
            mac[i] = hex_to_int(&str[i*3], 2);
        }
    }

    list_t createList() {
        list_t l = { NULL, NULL, 0 };

        return l;
    }

    void push(list_t* list, const uint8_t* mac) {
        item_t* i = (item_t*)malloc(sizeof(item_t));

        memcpy(i->data, mac, 6);
        i->next = NULL;

        if (list) {
            if (!list->begin) {
                list->begin = i;
                list->end   = i;
            } else {
                list->end->next = i;
                list->end       = i;
            }
            ++(list->size);
        }
    }

    void clear(list_t* list) {
        item_t* h = list->begin;

        while (h) {
            item_t* to_delete = h;
            h = h->next;
            free(to_delete);
        }

        list->begin = NULL;
        list->end   = NULL;
        list->size  = 0;
    }
}