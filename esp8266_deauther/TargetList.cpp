/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "TargetList.h"

#include <string.h>
#include <stdlib.h>

// ========== Target =========== //
typedef struct target_t {
    uint8_t   from[6];
    uint8_t   to[6];
    uint8_t   ch;
    target_t* next;
} target_t;

Target::Target(target_t* ptr) {
    this->ptr = ptr;
}

uint8_t* Target::from() const {
    if (ptr) return ptr->from;
    else return NULL;
}

uint8_t* Target::to() const {
    if (ptr) return ptr->to;
    else return NULL;
}

uint8_t Target::ch() const {
    if (ptr) return ptr->ch;
    else return 0;
}

bool Target::operator==(const Target& t) const {
    if (ptr == t.ptr) return true;
    if (!ptr) return false;

    return memcmp(from(), t.from(), 6) == 0 &&
           memcmp(to(), t.to(), 6) == 0 &&
           ch() == t.ch();
}

bool Target::operator<(const Target& t) const {
    if (ptr == t.ptr) return true;
    if (!ptr) return false;

    return memcmp(from(), t.from(), 6) < 0 ||
           memcmp(to(), t.to(), 6) < 0 ||
           ch() < t.ch();
}

bool Target::operator>(const Target& t) const {
    if (ptr == t.ptr) return true;
    if (!ptr) return false;

    return memcmp(from(), t.from(), 6) > 0 &&
           memcmp(to(), t.to(), 6) > 0 &&
           ch() > t.ch();
}

// ========== TargetList =========== //
TargetList::TargetList(int max) : list_max_size(max) {}

TargetList::~TargetList() {
    clear();
}

void TargetList::moveFrom(TargetList& t) {
    target_t* th = t.list_begin;

    while (th) {
        if ((list_max_size > 0) && (list_size >= list_max_size)) break;

        // Push to list
        if (!list_begin) {
            list_begin = th;
            list_end   = th;
            list_h     = list_begin;
        } else {
            list_end->next = th;
            list_end       = th;
        }

        ++(list_size);

        th = th->next;
    }

    t.list_begin = NULL;
    t.list_end   = NULL;
    t.list_size  = 0;
    t.list_h     = NULL;
    t.pos        = 0;
}

bool TargetList::push(const uint8_t* from, const uint8_t* to, const uint8_t ch) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    // Create new target
    target_t* new_target = (target_t*)malloc(sizeof(target_t));

    memcpy(new_target->from, from, 6);
    memcpy(new_target->to, to, 6);
    new_target->ch   = ch;
    new_target->next = NULL;

    Target t(new_target);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_target;
        list_end   = new_target;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (Target(list_begin) > t) {
            new_target->next = list_begin;
            list_begin       = new_target;
        }
        // Insert at end
        else if (Target(list_end) < t) {
            list_end->next = new_target;
            list_end       = new_target;
        }
        // Insert somewhere in the between (insertion sort)
        else {
            target_t* tmp_c = list_begin;
            target_t* tmp_p = NULL;

            int res;

            do {
                tmp_p = tmp_c;
                tmp_c = tmp_c->next;
            } while (tmp_c && Target(tmp_c) < t);


            // Skip duplicates
            if (res == 0) {
                free(new_target);
                return false;
            } else {
                new_target->next = tmp_c;
                if (tmp_p) tmp_p->next = new_target;
            }
        }
    }

    ++(list_size);
    return true;
}

Target TargetList::get(int i) {
    if (i < pos) {
        list_h = list_begin;
        pos    = 0;
    }

    while (list_h && pos<i) {
        list_h = list_h->next;
        ++pos;
    }

    return Target(list_h);
}

void TargetList::begin() {
    list_h = list_begin;
    pos    = 0;
}

Target TargetList::iterate() {
    target_t* tmp = list_h;

    if (list_h) {
        list_h = list_h->next;
        ++pos;
    }

    return Target(tmp);
}

bool TargetList::available() const {
    return list_h;
}

int TargetList::size() const {
    return list_size;
}

bool TargetList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}

void TargetList::clear() {
    list_h = list_begin;

    while (list_h) {
        target_t* to_delete = list_h;
        list_h = list_h->next;
        free(to_delete);
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    list_h = NULL;
    pos    = 0;
}