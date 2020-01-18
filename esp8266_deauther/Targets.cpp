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

Target::Target(target_t* ptr) {
    this->ptr = ptr;
}

uint8_t* Target::from() {
    if (ptr) return ptr->from;
    else return NULL;
}

uint8_t* Target::to() {
    if (ptr) return ptr->to;
    else return NULL;
}

uint8_t Target::ch() {
    if (ptr) return ptr->ch;
    else return 0;
}

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
    target_t* i = (target_t*)malloc(sizeof(target_t));

    memcpy(i->from, from, 6);
    memcpy(i->to, to, 6);
    i->ch   = ch;
    i->next = NULL;

    if (!list_begin) {
        list_begin = i;
        list_end   = i;
        h          = list_begin;
    } else {
        list_end->next = i;
        list_end       = i;
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