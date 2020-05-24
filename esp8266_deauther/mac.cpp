/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
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
    bool multicast(const uint8_t* mac) {
        return (mac[0] & 0x01) == 1;
    }

    bool equals(const uint8_t* macA, const uint8_t* macB) {
        for (uint8_t i = 0; i < 6; ++i) {
            if (macA[i] != macB[i]) return false;
        }

        return true;
    }

    bool valid(const char* str, unsigned int str_len, unsigned int len) {
        len = len - (len % 3);
        if (str_len < (len*2)+(len-1)) return false;

        for (int i = 0; i<len; ++i) {
            char first_char     = str[i*3];
            char second_char    = str[i*3+1];
            char delimiter_char = (i<(len-1)) ? str[i*3+2] : ':';

            bool first = (((first_char >= '0') && (first_char <= '9')) ||
                          ((first_char >= 'A') && (first_char <= 'Z')) ||
                          ((first_char >= 'a') && (first_char <= 'z')));

            bool second = (((second_char >= '0') && (second_char <= '9')) ||
                           ((second_char >= 'A') && (second_char <= 'Z')) ||
                           ((second_char >= 'a') && (second_char <= 'z')));

            bool delimiter = (/*i==5 || */ delimiter_char == ':');

            if (!first || !second || !delimiter) return false;
        }

        return true;
    }

    void fromStr(const char* str, uint8_t* mac, unsigned int len) {
        len = len - (len % 3);

        for (int i = 0; i<len; ++i) {
            mac[i] = hex_to_int(&str[i*3], 2);
        }
    }
}