/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

class StringList {
    private:
        typedef struct item_t {
            const char* ptr;
            int         len;
            item_t    * next;
        } item_t;

        item_t* list_begin = NULL;
        item_t* list_end   = NULL;
        int list_size      = 0;

        item_t* h = NULL;

    public:
        StringList(const String& input, String delimiter);
        ~StringList();

        String get(int i);
        String next();

        bool available();
        int size();
};