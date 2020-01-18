/*
   Based on Wireshark manufacturer database
   source: https://www.wireshark.org/tools/oui-lookup.html
   Wireshark is released under the GNU General Public License version 2
 */

#include "targets.h"

#include <cstring> // memcpy
#include <cstdlib> // free

typedef struct target_t {
    uint8_t   from[6];
    uint8_t   to[6];
    uint8_t   ch;
    target_t* next;
} target_t;

TargetList::~TargetList() {
    h = list_begin;

    while (h) {
        target_t* to_delete = h;
        h = h->next;
        free(to_delete);
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    h = NULL;
}

void TargetList::push(const uint8_t* from, const uint8_t* to, const uint8_t ch) {
    // Create new target
    target_t* new_target = (target_t*)malloc(sizeof(target_t));

    memcpy(new_target->from, from, 6);
    memcpy(new_target->to, to, 6);
    new_target->ch   = ch;
    new_target->next = NULL;

    // Check if already in list
    Target t(new_target);

    target_t* h = list_begin;

    while (h) {
        if (Target(h) == t) {
            free(new_target);
            return;
        }
        h = h->next;
    }

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
}

Target TargetList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && i<j) {
        h = h->next;
        ++j;
    }

    return Target(h);
}

Target TargetList::next() {
    Target t(h);

    if (h) {
        h = h->next;
    }

    return t;
}

bool TargetList::available() {
    return h;
}

int TargetList::size() {
    return list_size;
}

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