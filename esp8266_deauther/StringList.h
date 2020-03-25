/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h> // String

class StringList {
    protected:
        typedef struct str_t {
            char * ptr;
            str_t* next;
        } str_t;

        str_t* list_begin = NULL;
        str_t* list_end   = NULL;
        int list_size     = 0;
        int list_max_size;

        str_t* list_h = NULL;
        int list_pos  = 0;

        char* stringCopy(const char* str, unsigned long len) const;
        int compare(const str_t* a, const String& b) const;
        int compare(const str_t* a, const str_t* b) const;
        int compare(const str_t* a, const char* b) const;

    public:
        StringList(int max                                        = 0);
        StringList(const String& input, String delimiter, int max = 0);
        ~StringList();

        void moveFrom(StringList& sl);

        bool push(String str);
        virtual bool push(const char* str, unsigned long len);

        bool forcePush(String st);
        bool forcePush(const char* str, unsigned long len);

        void parse(const String& input, String delimiter);

        String get(int i);

        void begin();
        String iterate();

        bool contains(const String& str) const;
        virtual bool contains(const char* str) const;

        bool available() const;
        int size() const;
        bool full() const;

        void clear();
};

class SortedStringList : public StringList {
    public:
        SortedStringList(int max                                        = 0);
        SortedStringList(const String& input, String delimiter, int max = 0);

        using StringList::push;
        bool push(const char* str, unsigned long len) override;

        using StringList::contains;
        bool contains(const char* str) const override;
};