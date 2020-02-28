/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "strh.h"

namespace strh {
    String left(int len, String str) {
        int spaces = len - str.length();

        while (spaces > 0) {
            str = str + ' ';
            --spaces;
        }

        str = str.substring(0, len);

        return str;
    }

    String right(int len, String str) {
        int spaces = len - str.length();

        while (spaces > 0) {
            str = ' ' + str;
            --spaces;
        }

        str = str.substring(0, len);

        return str;
    }

    String center(int len, String str) {
        int spaces = len - str.length();

        for (int i = 0; i < spaces; i += 2) {
            str = ' ' + str + ' ';
        }

        str = str.substring(0, len);

        return str;
    }

    String mac(const uint8_t* b) {
        String str;

        for (int i = 0; i < 6; i++) {
            if (b[i] < 0x10) str += '0';
            str += String(b[i], HEX);

            if (i < 5) str += ':';
        }

        return str;
    }
}