/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

class StringList {
    protected:
        typedef struct item_t {
            char  * ptr;
            item_t* next;
        } item_t;

        item_t* list_begin = NULL;
        item_t* list_end   = NULL;
        int list_size      = 0;
        int list_max_size;

        item_t* h = NULL;

        char* stringCopy(const char* str, long len);

    public:
        StringList(int max = 0);
        StringList(const String& input, String delimiter);
        ~StringList();

        void moveFrom(StringList& sl);

        virtual bool push(String str);
        String popFirst();

        void parse(const String& input, String delimiter);

        String get(int i);

        void begin();
        String iterate();

        virtual bool contains(const String& str) const;
        bool available() const;
        int size() const;
        bool full() const;

        void clear();
};

class SortedStringList : public StringList {
    public:
        SortedStringList(int max = 0);

        bool push(String str) override;
        bool contains(const String& str) const override;
};