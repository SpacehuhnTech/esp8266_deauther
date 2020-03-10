/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "MACList.h"

// #include <string.h>
// #include <stdlib.h>

#include "StringList.h"
#include "mac.h"

// ========== MAC =========== //

MAC::MAC(const uint8_t* addr) {
    if (addr) memcpy(this->addr, addr, 6);
}

const uint8_t* MAC::getAddr() const {
    return addr;
}

MAC* MAC::getNext() {
    return next;
}

void MAC::setNext(MAC* next) {
    this->next = next;
}

// ========== MACList =========== //

int MACList::compare(const MAC* a, const uint8_t* b) const {
    return memcmp(a->getAddr(), b, 6);
}

int MACList::compare(const MAC* a, const MAC* b) const {
    return compare(a, b->getAddr());
}

MACList::MACList(int max) : list_max_size(max) {}

MACList::~MACList() {
    clear();
}

void MACList::parse(const String& input, String delimiter) {
    StringList mac_list(input, delimiter);

    mac_list.begin();

    while (mac_list.available()) {
        uint8_t mac[6];
        String  mac_str = mac_list.iterate();
        if (mac_str.length() == 17) {
            mac::fromStr(mac_str.c_str(), mac);
            push(mac);
        }
    }
}

bool MACList::push(const uint8_t* addr) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    // Create new target
    MAC* new_mac = new MAC(addr);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_mac;
        list_end   = new_mac;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (compare(list_begin, new_mac) > 0) {
            new_mac->setNext(list_begin);
            list_begin = new_mac;
        }
        // Insert at end
        else if (compare(list_end, new_mac) < 0) {
            list_end->setNext(new_mac);
            list_end = new_mac;
        }
        // Insert somewhere in the between (insertion sort)
        else {
            MAC* tmp_c = list_begin;
            MAC* tmp_p = NULL;

            int res = compare(tmp_c, new_mac);

            while (tmp_c->getNext() && res < 0) {
                tmp_p = tmp_c;
                tmp_c = tmp_c->getNext();
                res   = compare(tmp_c, new_mac);
            }

            // Skip duplicates
            if (res == 0) {
                delete new_mac;
                return false;
            } else {
                new_mac->setNext(tmp_c);
                if (tmp_p) tmp_p->setNext(new_mac);
            }
        }
    }

    ++(list_size);
    return true;
}

MAC* MACList::search(const uint8_t* mac) {
    if ((list_size == 0) || (compare(list_begin, mac) > 0) || (compare(list_end, mac) < 0)) {
        return NULL;
    }

    MAC* tmp = list_begin;
    int  res = compare(tmp, mac);

    while (tmp->getNext() && res < 0) {
        tmp = tmp->getNext();
        res = compare(tmp, mac);
    }

    return (res == 0) ? tmp : NULL;
}

void MACList::clear() {
    list_h = list_begin;

    while (list_h) {
        MAC* to_delete = list_h;
        list_h = list_h->getNext();
        delete to_delete;
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    list_h   = NULL;
    list_pos = 0;
}

MAC* MACList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return list_h;
}

void MACList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

MAC* MACList::iterate() {
    MAC* tmp = list_h;

    if (list_h) {
        list_h = list_h->getNext();
        ++list_pos;
    }

    return tmp;
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