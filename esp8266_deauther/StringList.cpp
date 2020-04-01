/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "StringList.h"

// ========== StringList ========== //

char* StringList::stringCopy(const char* str, unsigned long len) const {
    char* newstr = new char[len+1];

    memcpy(newstr, str, len);
    newstr[len] = '\0';

    return newstr;
}

int StringList::compare(const str_t* a, const String& b) const {
    return strcmp(a->ptr, b.c_str());
}

int StringList::compare(const str_t* a, const str_t* b) const {
    return strcmp(a->ptr, b->ptr);
}

int StringList::compare(const str_t* a, const char* b) const {
    return strcmp(a->ptr, b);
}

StringList::StringList(int max) : list_max_size(max) {}

StringList::StringList(const String& input, String delimiter, int max) : list_max_size(max) {
    parse(input, delimiter);
}

StringList::~StringList() {
    clear();
}

void StringList::moveFrom(StringList& sl) {
    str_t* ih = sl.list_begin;

    while (ih) {
        if ((list_max_size > 0) && (list_size >= list_max_size)) break;

        // Push to list
        if (!list_begin) {
            list_begin = ih;
            list_end   = ih;
            list_p     = nullptr;
            list_h     = list_begin;
        } else {
            list_end->next = ih;
            list_end       = ih;
        }

        ++(list_size);

        ih = ih->next;
    }

    sl.list_begin = nullptr;
    sl.list_end   = nullptr;
    sl.list_size  = 0;
    sl.list_p     = nullptr;
    sl.list_h     = nullptr;
    sl.list_pos   = 0;
}

bool StringList::push(String str) {
    return push(str.c_str(), str.length());
}

bool StringList::push(const char* str, unsigned long len) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    str_t* new_str = new str_t();

    new_str->ptr  = stringCopy(str, len);
    new_str->next = nullptr;

    if (!list_begin) {
        list_begin = new_str;
        list_end   = new_str;
        list_p     = nullptr;
        list_h     = list_begin;
    } else {
        list_end->next = new_str;
        list_end       = new_str;
    }

    ++list_size;
    return true;
}

bool StringList::forcePush(String str) {
    return forcePush(str.c_str(), str.length());
}

bool StringList::forcePush(const char* str, unsigned long len) {
    if (!push(str, len)) {
        str_t* new_str = list_begin;

        list_begin = list_begin->next;

        delete[] new_str->ptr;
        new_str->ptr  = stringCopy(str, len);
        new_str->next = nullptr;

        list_end->next = new_str;
        list_end       = new_str;
    }
    return true;
}

void StringList::parse(const String& input, String delimiter) {
    int len           = input.length();
    int delimiter_len = delimiter.length();
    int j             = 0;
    const char* ptr   = input.c_str();

    for (int i = 0; i <= len; ++i) {
        if ((i-j > 0) && ((i == len) || (input.substring(i, i+delimiter_len) == delimiter))) {
            push(&ptr[j], i-j);
            j = i+delimiter_len;
        }
    }
}

String StringList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return String(list_h ? list_h->ptr : "");
}

void StringList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

String StringList::iterate() {
    String res(list_h ? list_h->ptr : "");

    if (list_h) {
        list_p = list_h;
        list_h = list_h->next;
        ++list_pos;
    }

    return res;
}

void StringList::remove() {
    if (list_h) {
        if (list_p) {
            list_p->next = list_h->next;
            delete list_h;
            list_h = list_p->next;
        } else {
            list_begin = list_h->next;
            delete list_h;
            list_h = list_begin;
        }
        --list_size;
    }
}

bool StringList::contains(const String& str) const {
    return contains(str.c_str());
}

bool StringList::contains(const char* str) const {
    str_t* tmp = list_begin;

    while (tmp && compare(tmp, str) != 0) {
        tmp = tmp->next;
    }

    return tmp;
}

bool StringList::available() const {
    return list_h;
}

int StringList::size() const {
    return list_size;
}

bool StringList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}

void StringList::clear() {
    str_t* tmp = list_begin;

    while (tmp) {
        str_t* to_delete = tmp;
        tmp = tmp->next;
        delete[] to_delete->ptr;
        delete to_delete;
    }

    list_begin = nullptr;
    list_end   = nullptr;
    list_size  = 0;

    list_p   = nullptr;
    list_h   = nullptr;
    list_pos = 0;
}

// ========== SortedStringList ========== //

SortedStringList::SortedStringList(int max) : StringList(max) {}

SortedStringList::SortedStringList(const String& input, String delimiter, int max) : StringList(max) {
    parse(input, delimiter);
}

bool SortedStringList::push(const char* str, unsigned long len) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    str_t* new_str = new str_t();

    new_str->ptr  = stringCopy(str, len);
    new_str->next = nullptr;

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_str;
        list_end   = new_str;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (compare(list_begin, new_str) > 0) {
            new_str->next = list_begin;
            list_begin    = new_str;
        }
        // Insert at end
        else if (compare(list_end, new_str) < 0) {
            list_end->next = new_str;
            list_end       = new_str;
        }
        // Insert somewhere inbetween (insertion sort)
        else {
            str_t* tmp_c = list_begin;
            str_t* tmp_p = nullptr;

            int res = compare(tmp_c, new_str);

            while (tmp_c->next && res < 0) {
                tmp_p = tmp_c;
                tmp_c = tmp_c->next;
                res   = compare(tmp_c, new_str);
            }

            // Skip duplicates
            if (res == 0) {
                delete[] new_str->ptr;
                delete new_str;
                return false;
            } else {
                new_str->next = tmp_c;
                tmp_p->next   = new_str;
            }
        }
    }

    ++list_size;
    return true;
}

bool SortedStringList::contains(const char* str) const {
    if ((list_size == 0) || (compare(list_begin, str) > 0) || (compare(list_end, str) < 0)) {
        return false;
    }

    str_t* tmp = list_begin;

    int res = compare(tmp, str);

    while (tmp->next && res < 0) {
        tmp = tmp->next;
        res = compare(tmp, str);
    }

    return res == 0;
}