/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "TargetList.h"

#include <string.h>
#include <stdlib.h>

// ========== Target =========== //
Target::Target(const uint8_t* from, const uint8_t* to, uint8_t ch) {
    memcpy(this->from, from, 6);
    memcpy(this->to, to, 6);
    this->ch   = ch;
    this->next = nullptr;
}

const uint8_t* Target::getFrom() const {
    return from;
}

const uint8_t* Target::getTo() const {
    return to;
}

uint8_t Target::getCh() const {
    return ch;
}

Target* Target::getNext() {
    return this->next;
}

void Target::setNext(Target* next) {
    this->next = next;
}

// ========== TargetList =========== //

int TargetList::compare(const Target* a, const Target* b) const {
    if (a == b) return 0;
    else if ((memcmp(a->getFrom(), b->getFrom(), 6) < 0) ||
             (memcmp(a->getTo(), b->getTo(), 6) < 0) ||
             (a->getCh() < b->getCh())) return -1;
    else if ((memcmp(a->getFrom(), b->getFrom(), 6) > 0) ||
             (memcmp(a->getTo(), b->getTo(), 6) > 0) ||
             (a->getCh() > b->getCh())) return 1;
    else return 0;
}

TargetList::TargetList(int max) : list_max_size(max) {}

TargetList::~TargetList() {
    clear();
}

void TargetList::moveFrom(TargetList& t) {
    Target* tmp = t.list_begin;

    while (tmp) {
        if ((list_max_size > 0) && (list_size >= list_max_size)) break;

        // Push to list
        if (!list_begin) {
            list_begin = tmp;
            list_end   = tmp;
            list_h     = list_begin;
        } else {
            list_end->setNext(tmp);
            list_end = tmp;
        }

        ++(list_size);

        tmp = tmp->getNext();
    }

    t.list_begin = nullptr;
    t.list_end   = nullptr;
    t.list_size  = 0;
    t.list_h     = nullptr;
    t.list_pos   = 0;
}

bool TargetList::push(const uint8_t* from, const uint8_t* to, const uint8_t ch) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    // Create new target
    Target* new_target = new Target(from, to, ch);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_target;
        list_end   = new_target;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (compare(list_begin, new_target) > 0) {
            new_target->setNext(list_begin);
            list_begin = new_target;
        }
        // Insert at end
        else if (compare(list_end, new_target) < 0) {
            list_end->setNext(new_target);
            list_end = new_target;
        }
        // Insert somewhere in the between (insertion sort)
        else {
            Target* tmp_c = list_begin;
            Target* tmp_p = nullptr;

            int res = compare(tmp_c, new_target);

            while (tmp_c->getNext() && res < 0) {
                tmp_p = tmp_c;
                tmp_c = tmp_c->getNext();
                res   = compare(tmp_c, new_target);
            }

            // Skip duplicates
            if (res == 0) {
                free(new_target);
                return false;
            } else {
                new_target->setNext(tmp_c);
                if (tmp_p) tmp_p->setNext(new_target);
            }
        }
    }

    ++(list_size);
    return true;
}

Target* TargetList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return list_h;
}

void TargetList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

Target* TargetList::iterate() {
    Target* tmp = list_h;

    if (list_h) {
        list_h = list_h->getNext();
        ++list_pos;
    }

    return tmp;
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
    Target* tmp = list_begin;

    while (tmp) {
        Target* to_delete = tmp;
        tmp = tmp->getNext();
        delete tmp;
    }

    list_begin = nullptr;
    list_end   = nullptr;
    list_size  = 0;

    list_h   = nullptr;
    list_pos = 0;
}