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

        typedef struct str_list_t {
            str_t* begin;
            str_t* end;

            unsigned int size;
            unsigned int max;

            str_t* p;
            str_t* h;

            unsigned int pos;
        } str_list_t;

        str_list_t list { nullptr, nullptr, 0, 0, nullptr, nullptr, 0 };

        str_t* str_copy(const char* ptr, unsigned int len);

    public:
        StringList(unsigned int max                      = 0);
        StringList(const String& input, String delimiter = ",");

        StringList(const StringList& sl);
        StringList(StringList&& sl);

        ~StringList();

        StringList& operator=(const StringList& sl);
        StringList& operator=(StringList&& sl);

        void clear();

        void parse(const String& input, String delimiter = ",");

        bool push(const String& str);
        virtual bool push(const char* str, unsigned long len);

        String get(unsigned int pos);

        void begin();
        String iterate();
        void remove();

        bool available() const;
        int size() const;
        bool full() const;
        bool empty() const;

        bool contains(const String& str, bool case_sensitive = true) const;
        virtual bool contains(const char* str, bool case_sensitive = true) const;
};

class SortedStringList : public StringList {
    public:
        using StringList::StringList;

        using StringList::push;
        bool push(const char* str, unsigned long len) override;

        using StringList::get;

        using StringList::contains;
        bool contains(const char* str, bool case_sensitive = true) const override;
};