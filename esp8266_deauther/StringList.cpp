/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "StringList.h"

#include "config.h"

#ifdef DEBUG_STRING_LIST
#include "debug.h"
#include "strh.h"
#else // ifdef DEBUG_STRING_LIST
#define debug(...) 0
#define debugln(...) 0
#define debugf(...) 0
#define debugF(...) 0
#define debuglnF(...) 0
#endif // ifdef DEBUG_STRING_LIST

// ========== StringList ========== //

// ===== PRIVATE ===== //

StringList::str_t* StringList::str_copy(const char* ptr, unsigned int len) {
    StringList::str_t* new_str = new StringList::str_t;

    new_str->ptr = new char[len+1];
    memcpy(new_str->ptr, ptr, len);
    new_str->ptr[len] = '\0';
    new_str->next     = nullptr;

    return new_str;
}

// ===== PUBLIC ===== //

StringList::StringList(unsigned int max) {
    list.max = max;
}

StringList::StringList(const String& input, String delimiter) {
    parse(input, delimiter);
}

StringList::StringList(const StringList& sl) {
    if (this != &sl) {
        clear();

        str_t* tmp = sl.list.begin;

        while (tmp) {
            push(tmp->ptr, strlen(tmp->ptr));
            tmp = tmp->next;
        }
    }
}

StringList::StringList(StringList&& sl) {
    if (this != &sl) {
        clear();

        list = sl.list;

        sl.list.begin = nullptr;
        sl.list.end   = nullptr;
        sl.list.size  = 0;
        sl.list.p     = nullptr;
        sl.list.h     = nullptr;
        sl.list.pos   = 0;
    }
}

StringList::~StringList() {
    clear();
    debuglnF("[StringList] destroyed");
}

StringList& StringList::operator=(const StringList& sl) {
    debuglnF("[StringList] copy assignment");

    if (this != &sl) {
        clear();

        str_t* tmp = sl.list.begin;

        while (tmp) {
            push(tmp->ptr, strlen(tmp->ptr));
            tmp = tmp->next;
        }
    }
    return *this;
}

StringList& StringList::operator=(StringList&& sl) {
    debuglnF("[StringList] move assignment");

    if (this != &sl) {
        clear();

        list = sl.list;

        sl.list.begin = nullptr;
        sl.list.end   = nullptr;
        sl.list.size  = 0;
        sl.list.p     = nullptr;
        sl.list.h     = nullptr;
        sl.list.pos   = 0;
    }

    return *this;
}

void StringList::clear() {
    debuglnF("[StringList] clear");

    begin();

    while (available()) {
        remove();
    }

    list.begin = nullptr;
    list.end   = nullptr;
    list.size  = 0;
    list.p     = nullptr;
    list.h     = nullptr;
    list.pos   = 0;
}

void StringList::parse(const String& input, String delimiter) {
    debuglnF("[StringList] parsing input");

    unsigned int len           = input.length();
    unsigned int delimiter_len = delimiter.length();
    unsigned int j             = 0;
    const char * ptr           = input.c_str();

    for (int i = 0; i <= len; ++i) {
        if ((i-j > 0) && ((i == len) || (input.substring(i, i+delimiter_len) == delimiter))) {
            push(&ptr[j], i-j);
            j = i+delimiter_len;
        }
    }
}

bool StringList::push(const String& str) {
    return push(str.c_str(), str.length());
}

bool StringList::push(const char* str, unsigned long len) {
    debugF("[StringList] push...");

    if (full()) {
        debuglnF("full");
        return false;
    }

    str_t* new_str = str_copy(str, len);

    if (empty()) {
        list.begin = new_str;
        list.end   = new_str;
        begin();
    } else {
        list.end->next = new_str;
        list.end       = new_str;
    }

    ++list.size;

    debuglnF("OK");

    return true;
}

String StringList::get(unsigned int pos) {
    if (empty() || (pos >= list.size)) return String{};

    debuglnF("[StringList] get");

    if (list.pos > pos) begin();

    while (list.pos < pos) {
        iterate();
    }

    return String{ list.h->ptr };
}

void StringList::begin() {
    list.p   = nullptr;
    list.h   = list.begin;
    list.pos = 0;
}

String StringList::iterate() {
    if (list.size == 0) return String{};
    if (!available()) begin();

    if (list.h) {
        list.p = list.h;
        list.h = list.h->next;
        ++list.pos;
        return String{ list.p->ptr };
    }

    return String{};
}

void StringList::remove() {
    debuglnF("[StringList] remove");

    if (list.h) {
        if (list.p) {
            list.p->next = list.h->next;
            delete[] list.h->ptr;
            delete list.h;
            list.h = list.p->next;
        } else {
            list.begin = list.h->next;
            delete[] list.h->ptr;
            delete list.h;
            list.h = list.begin;
        }
        --list.size;
    }
}

bool StringList::available() const {
    return list.h;
}

int StringList::size() const {
    return list.size;
}

bool StringList::full() const {
    return list.max > 0 && list.size >= list.max;
}

bool StringList::empty() const {
    return list.size == 0;
}

bool StringList::contains(const String& str, bool case_sensitive) const {
    return contains(str.c_str(), case_sensitive);
}

bool StringList::contains(const char* str, bool case_sensitive) const {
    debuglnF("[StringList] contains");

    str_t* tmp = list.begin;

    while (tmp && (case_sensitive ? (strcmp(str, tmp->ptr) != 0) : (strcasecmp(str, tmp->ptr) != 0))) {
        tmp = tmp->next;
    }

    return tmp;
}

// ========== SortedStringList ========== //

bool SortedStringList::push(const char* str, unsigned long len) {
    debugF("[SortedStringList] push...");
    debug(str);

    if (full()) {
        debuglnF("full");
        return false;
    }

    str_t* new_str = str_copy(str, len);

    // Empty list -> insert first element
    if (empty()) {
        list.begin = new_str;
        list.end   = new_str;
        debugF("empty list...");
    } else {
        // Insert at start
        if (strcmp(list.begin->ptr, new_str->ptr) > 0) {
            new_str->next = list.begin;
            list.begin    = new_str;
            debugF("insert at start...");
        }
        // Insert at end
        else if (strcmp(list.end->ptr, new_str->ptr) < 0) {
            list.end->next = new_str;
            list.end       = new_str;
            debugF("insert at end...");
        }
        // Insert somewhere inbetween (insertion sort)
        else {
            begin();

            unsigned int i = 0;
            int res        = strcmp(list.h->ptr, new_str->ptr);

            while (available() && res < 0) {
                iterate();
                res = strcmp(list.h->ptr, new_str->ptr);
                ++i;
            }

            // Skip duplicates
            if (res == 0) {
                debuglnF("duplicate");
                delete[] new_str->ptr;
                delete new_str;
                return false;
            }

            new_str->next = list.h;
            list.p->next  = new_str;

            debugF("insert at ");
            debug(i);
            debugF("...");
        }
    }

    ++list.size;
    begin();

    debuglnF("OK");

    return true;
}

bool SortedStringList::contains(const char* str, bool case_sensitive) const {
    debuglnF("[SortedStringList] contains");

    if (empty() ||
        (strcasecmp(list.begin->ptr, str) > 0) ||
        (strcasecmp(list.end->ptr, str) < 0)) {
        return false;
    }

    str_t* tmp = list.begin;

    int res = strcasecmp(tmp->ptr, str);

    while (tmp->next && res < 0) {
        tmp = tmp->next;
        res = case_sensitive ? strcmp(tmp->ptr, str) : strcasecmp(tmp->ptr, str);
    }

    return res == 0;
}