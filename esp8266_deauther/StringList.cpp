/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "StringList.h"

char* StringList::stringCopy(const char* str, long len) {
    char* newstr = (char*)malloc(len+1);

    memcpy(newstr, str, len);
    newstr[len] = '\0';

    return newstr;
}

StringList::StringList() {}

StringList::StringList(const String& input, String delimiter) {
    parse(input, delimiter);
}

StringList::~StringList() {
    clear();
}

void StringList::moveFrom(StringList& sl) {
    item_t* ih = sl.list_begin;

    while (ih) {
        // Push to list
        if (!list_begin) {
            list_begin = ih;
            list_end   = ih;
            h          = list_begin;
        } else {
            list_end->next = ih;
            list_end       = ih;
        }

        ++(list_size);

        ih = ih->next;
    }

    sl.list_begin = NULL;
    sl.list_end   = NULL;
    sl.list_size  = 0;
    sl.h          = NULL;
}

void StringList::push(String str) {
    item_t* item = (item_t*)malloc(sizeof(item_t));

    item->ptr  = stringCopy(str.c_str(), str.length());
    item->next = NULL;

    if (!list_begin) {
        list_begin = item;
        list_end   = item;
        h          = list_begin;
    } else {
        list_end->next = item;
        list_end       = item;
    }

    ++list_size;
}

String StringList::popFirst() {
    String str(list_begin->ptr);

    item_t* next = list_begin->next;

    free(list_begin);

    if (next) {
        list_begin = next;
        h          = list_begin;
    } else {
        list_begin = NULL;
        list_end   = NULL;
        h          = NULL;
    }

    return str;
}

void StringList::parse(const String& input, String delimiter) {
    int len           = input.length();
    int delimiter_len = delimiter.length();
    int j             = 0;
    const char* ptr   = input.c_str();

    for (int i = 0; i <= len; ++i) {
        if ((i-j > 0) && ((i == len) || (input.substring(i, i+delimiter_len) == delimiter))) {
            item_t* item = (item_t*)malloc(sizeof(item_t));
            item->ptr  = stringCopy(&ptr[j], i-j);
            item->next = NULL;

            j = i+delimiter_len;

            if (!list_begin) {
                list_begin = item;
                list_end   = item;
                h          = list_begin;
            } else {
                list_end->next = item;
                list_end       = item;
            }

            ++list_size;
        }
    }
}

String StringList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && i<j) {
        h = h->next;
        ++j;
    }

    return String(h ? h->ptr : "");
}

void StringList::begin() {
    h = list_begin;
}

String StringList::iterate() {
    String res(h ? h->ptr : "");

    h = h->next;

    return res;
}

bool StringList::contains(const String& str) const {
    item_t* h = list_begin;

    while (h && strcmp(h->ptr, str.c_str()) != 0) {
        h = h->next;
    }
    return h;
}

bool StringList::available() const {
    return h;
}

int StringList::size() const {
    return list_size;
}

void StringList::clear() {
    h = list_begin;

    while (h) {
        item_t* to_delete = h;
        h = h->next;
        free(to_delete->ptr);
        free(to_delete);
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    h = NULL;
}

void SortedStringList::push(String str) {
    item_t* item = (item_t*)malloc(sizeof(item_t));

    item->ptr  = stringCopy(str.c_str(), str.length());
    item->next = NULL;

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = item;
        list_end   = item;
        h          = list_begin;
    } else {
        // Insert at start
        if (strcmp(list_begin->ptr, item->ptr) > 0) {
            item->next = list_begin;
            list_begin = item;
        }
        // Insert at end
        else if (strcmp(list_end->ptr, item->ptr) < 0) {
            list_end->next = item;
            list_end       = item;
        }
        // Insert somewhere in the middle (insertion sort)
        else {
            item_t* tmp_c = list_begin;
            item_t* tmp_p = NULL;

            while (tmp_c && strcmp(tmp_c->ptr, item->ptr) < 0) {
                tmp_p = tmp_c;
                tmp_c = tmp_c->next;
            }

            item->next = tmp_c;
            if (tmp_p) tmp_p->next = item;
        }
    }

    ++list_size;
}

bool SortedStringList::contains(const String& str) const {
    if (list_begin) {
        item_t* h = list_begin;
        int     res;

        do {
            res = strcmp(h->ptr, str.c_str());
            if (res == 0) return true;
            else h = h->next;
        } while (h && res < 0);
    }

    return false;
}