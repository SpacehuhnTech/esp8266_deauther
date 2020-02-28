/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "MACList.h"

#include <string.h>
#include <stdlib.h>

// ========== MACList =========== //

int MACList::compare(const mac_t* a, const uint8_t* b) const {
    return memcmp(a->addr, b, 6);
}

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
}

bool MACList::push(const uint8_t* addr) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    // Create new target
    mac_t* new_target = (mac_t*)malloc(sizeof(mac_t));

    memcpy(new_target->addr, addr, 6);
    new_target->next = NULL;

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_target;
        list_end   = new_target;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (compare(list_begin, addr) > 0) {
            new_target->next = list_begin;
            list_begin       = new_target;
        }
        // Insert at end
        else if (compare(list_end, addr) < 0) {
            list_end->next = new_target;
            list_end       = new_target;
        }
        // Insert somewhere in the between (insertion sort)
        else {
            mac_t* tmp_c = list_begin;
            mac_t* tmp_p = NULL;

            int res;

            do {
                res   = compare(tmp_c, addr);
                tmp_p = tmp_c;
                tmp_c = tmp_c->next;
            } while (tmp_c && res < 0);

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

uint8_t* MACList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return list_h ? list_h->addr : NULL;
}

void MACList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

uint8_t* MACList::iterate() {
    if (list_h) {
        mac_t* tmp = list_h;
        list_h = list_h->next;
        ++list_pos;
        return tmp->addr;
    } else {
        return NULL;
    }
}

bool MACList::available() const {
    return list_h;
}

int MACList::size() const {
    return list_size;
}

bool MACList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}

bool MACList::contains(const uint8_t* mac) const {
    if ((list_size == 0) || (compare(list_begin, mac) > 0) || (compare(list_end, mac) < 0)) {
        return false;
    }

    mac_t* tmp = list_begin;
    int    res;

    while (tmp && res < 0) {
        res = compare(tmp, mac);
        tmp = tmp->next;
    }

    return res == 0;
}

void MACList::clear() {
    list_h = list_begin;

    while (list_h) {
        mac_t* to_delete = list_h;
        list_h = list_h->next;
        free(to_delete);
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    list_h   = NULL;
    list_pos = 0;
}