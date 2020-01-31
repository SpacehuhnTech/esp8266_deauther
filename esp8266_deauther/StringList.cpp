/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "StringList.h"

StringList::StringList() {}

StringList::StringList(const String& input, String delimiter) {
    parse(input, delimiter);
}

StringList::~StringList() {
    h = list_begin;

    while (h) {
        item_t* to_delete = h;
        h = h->next;
        free(to_delete);
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    h = NULL;
}

void StringList::parse(const String& input, String delimiter) {
    int len           = input.length();
    int delimiter_len = delimiter.length();
    int j             = 0;
    const char* ptr   = input.c_str();

    for (int i = 0; i <= len; ++i) {
        if ((i-j > 0) && ((i == len) || (input.substring(i, i+delimiter_len) == delimiter))) {
            item_t* item = (item_t*)malloc(sizeof(item_t));
            item->ptr  = &ptr[j];
            item->len  = i-j;
            item->next = NULL;

            j = i+delimiter_len;

            if (!list_begin) {
                list_begin = item;
                list_end   = item;
            } else {
                list_end->next = item;
                list_end       = item;
            }

            ++list_size;
        }
    }

    h = list_begin;
}

String StringList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && i<j) {
        h = h->next;
        ++j;
    }

    String res;

    if (h) {
        for (int j = 0; j<h->len; ++j) {
            res += char(h->ptr[j]);
        }
    }

    return res;
}

void StringList::begin() {
    h = list_begin;
}

String StringList::iterate() {
    String res;

    if (h) {
        for (int i = 0; i<h->len; ++i) {
            res += char(h->ptr[i]);
        }
        h = h->next;
    }

    return res;
}

bool StringList::available() const {
    return h;
}

int StringList::size() const {
    return list_size;
}