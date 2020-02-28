/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "MACList.h"

#include <string.h>
#include <stdlib.h>

// ========== Target =========== //
typedef struct mac_t {
    uint8_t addr[6];
    mac_t * next;
} mac_t;

// ========== MACList =========== //
MACList::MACList(int max) : list_max_size(max) {}

MACList::~MACList() {
    clear();
}

void MACList::moveFrom(MACList& t) {
    mac_t* th = t.list_begin;

    while (th) {
        if ((list_max_size > 0) && (list_size >= list_max_size)) break;

        // Push to list
        if (!list_begin) {
            list_begin = th;
            list_end   = th;
            h          = list_begin;
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
    t.h          = NULL;
}

bool MACList::push(const uint8_t* addr) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    // Create new target
    mac_t* new_target = (mac_t*)malloc(sizeof(mac_t));

    memcpy(new_target->addr, addr, 6);
    new_target->next = NULL;

    // Check if already in list

    /*
       mac_t* h = list_begin;

       while (h) {
        if (Target(h) == t) {
            free(new_target);
            return false;
        }
        h = h->next;
       }*/

    // Push to list
    if (!list_begin) {
        list_begin = new_target;
        list_end   = new_target;
        h          = list_begin;
    } else {
        list_end->next = new_target;
        list_end       = new_target;
    }

    ++(list_size);
    return true;
}

uint8_t* MACList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && j<i) {
        h = h->next;
        ++j;
    }

    return iterate();
}

void MACList::begin() {
    h = list_begin;
}

uint8_t* MACList::iterate() {
    if (h) {
        mac_t* tmp = h;
        h = h->next;
        return tmp->addr;
    } else {
        return NULL;
    }
}

bool MACList::available() const {
    return h;
}

int MACList::size() const {
    return list_size;
}

bool MACList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}

void MACList::clear() {
    h = list_begin;

    while (h) {
        mac_t* to_delete = h;
        h = h->next;
        free(to_delete);
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    h = NULL;
}