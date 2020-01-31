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
            char  * ptr;
            item_t* next;
        } item_t;

        item_t* list_begin = NULL;
        item_t* list_end   = NULL;
        int list_size      = 0;

        item_t* h = NULL;

        char* stringCopy(const char* str, long len);

    public:
        StringList();
        StringList(const String& input, String delimiter);
        ~StringList();

        void parse(const String& input, String delimiter);

        String get(int i);

        void begin();
        String iterate();

        bool available() const;
        int size() const;
};