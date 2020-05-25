/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "TargetArr.h"
#include "mac.h"

#ifdef DEBUG_TARGET_ARR
#include "debug.h"
#include "strh.h"
#else // ifdef DEBUG_TARGET_ARR
#define debug(...) 0
#define debugln(...) 0
#define debugf(...) 0
#define debugF(...) 0
#define debuglnF(...) 0
#endif // ifdef DEBUG_TARGET_ARR

// ===== PRIVATE ===== //

bool TargetArr::contains(const target_t& t) {
    for (unsigned int i = 0; i<list.size; ++i) {
        if ((list.data[i].channels == t.channels) &&
            (memcmp(list.data[i].sender, t.sender, 6) == 0) &&
            (memcmp(list.data[i].receiver, t.receiver, 6) == 0)) return true;
    }
    return false;
}

// ===== PUBLIC ===== //

TargetArr::TargetArr(int size) {
    if (size > 0) {
        list.data = new target_t[size];
        list.max  = size;
    }
}

TargetArr::TargetArr(const TargetArr& tl) {
    if (tl.list.size > 0) {
        list.data = new target_t[tl.list.size];

        memcpy(list.data, tl.list.data, sizeof(target_t)*tl.list.size);

        list.size = tl.list.size;
        list.max  = tl.list.size;
        list.h    = tl.list.h;
    }
}

TargetArr::TargetArr(TargetArr&& tl) {
    list = tl.list;

    tl.list.data = nullptr;
    tl.clear();
}

TargetArr::~TargetArr() {
    clear();
    debuglnF("[TargetArr] destroyed");
}

TargetArr& TargetArr::operator=(const TargetArr& tl) {
    debuglnF("[TargetArr] copy assignment");

    if (this != &tl) {
        clear();

        if (tl.list.size > 0) {
            list.data = new target_t[tl.list.size];

            memcpy(list.data, tl.list.data, sizeof(target_t)*tl.list.size);

            list.size = tl.list.size;
            list.max  = tl.list.size;
            list.h    = tl.list.h;
        }
    }

    return *this;
}

TargetArr& TargetArr::operator=(TargetArr&& tl) {
    debuglnF("[TargetArr] move assignment");

    if (this != &tl) {
        clear();

        list = tl.list;

        tl.list.data = nullptr;
        tl.clear();
    }

    return *this;
}

TargetArr& TargetArr::operator+=(const TargetArr& tl) {
    debuglnF("[TargetArr] += operation begin");

    if ((this != &tl) && (tl.list.size > 0)) {
        // New list without data and max set to current size
        target_list_t new_list { nullptr, list.size, 0, 0 };

        // Count new data
        for (unsigned int i = 0; i<tl.list.size; ++i) {
            if (!contains(tl.list.data[i])) {
                ++new_list.max;
            }
        }

        // Create new buffer
        new_list.data = new target_t[new_list.max];

        // Copy current data
        memcpy(new_list.data, list.data, sizeof(target_t)*list.size);
        new_list.size = list.size;

        // Delete current data
        clear();

        // Replace list with new list
        list = new_list;

        // Copy new data
        for (unsigned int i = 0; i<tl.list.size; ++i) {
            add(tl.list.data[i].sender,
                tl.list.data[i].receiver,
                tl.list.data[i].channels);
        }
    }

    debuglnF("[TargetArr] += operation end");

    return *this;
}

void TargetArr::clear() {
    debuglnF("[TargetArr] clear");

    if (list.data) {
        delete[] list.data;
        list.data = nullptr;
    }

    list.size = 0;
    list.h    = 0;
    list.max  = 0;
}

bool TargetArr::add(const uint8_t* sender, const uint8_t* receiver, uint16_t channels) {
    debugF("[TargetArr] adding target...");

    if (list.size >= list.max) {
        debuglnF("list full");
        return false;
    }

    target_t* new_target = &list.data[list.size];

    memcpy(new_target->sender, sender, 6);
    memcpy(new_target->receiver, receiver, 6);
    new_target->channels = channels & 0x3FFF;

    if (contains(*new_target)) {
        debuglnF("already in list");
        return false;
    } else {
        debuglnF("OK");
        ++list.size;
        return true;
    }
}

bool TargetArr::add(const AccessPoint* ap) {
    if (!ap) return false;

    const uint8_t* sender { ap->getBSSID() };
    const uint8_t* receiver { mac::BROADCAST };
    uint16_t channels = 1 << (ap->getChannel()-1);

    return add(sender, receiver, channels);
}

bool TargetArr::add(const Station* st) {
    if (!st) return false;

    const AccessPoint* ap { st->getAccessPoint() };

    if (!ap) return false;

    const uint8_t* sender   { ap->getBSSID() };
    const uint8_t* receiver { st->getMAC() };
    uint16_t channels = 1 << (ap->getChannel()-1);

    return add(sender, receiver, channels);
}

void TargetArr::begin() {
    list.h = 0;
}

const target_t* TargetArr::iterate() {
    if (list.size == 0) return nullptr;
    if (!available()) list.h = 0;
    return &list.data[list.h++];
}

bool TargetArr::available() const {
    return list.h < list.size;
}

int TargetArr::size() const {
    return list.size;
}

bool TargetArr::full() const {
    return list.size >= list.max;
}