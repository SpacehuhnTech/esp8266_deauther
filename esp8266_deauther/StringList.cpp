/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "StringList.h"

StringList::StringList(const String& input, String delimiter) {
    int len           = input.length();
    int delimiter_len = delimiter.length();
    int j             = 0;
    const char* ptr   = input.c_str();

    for (int i = 0; i <= len; ++i) {
        if ((i-j > 0) && ((i == len) || ((i+delimiter_len < len) && (input.substring(i, i+delimiter_len) == delimiter)))) {
            item_t* item = (item_t*)malloc(sizeof(item_t));
            item->ptr  = &ptr[j];
            item->len  = i-j;
            item->next = NULL;

            j = i+delimiter_len;

            if (!begin) {
                begin = item;
                end   = item;
            } else {
                end->next = item;
                end       = item;
            }

            ++size;
        }
    }

    h = begin;
}

StringList::~StringList() {
    h = begin;

    while (h) {
        item_t* to_delete = h;
        h = h->next;
        free(to_delete);
    }

    begin = NULL;
    end   = NULL;
    size  = 0;

    h = NULL;
}

String StringList::get(int i) {
    h = begin;
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

String StringList::next() {
    String res;

    if (h) {
        for (int i = 0; i<h->len; ++i) {
            res += char(h->ptr[i]);
        }
        h = h->next;
    }

    return res;
}

bool StringList::available() {
    return h;
}