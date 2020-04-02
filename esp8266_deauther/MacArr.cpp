/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "MacArr.h"

// #include <string.h>
// #include <stdlib.h>

#include "StringList.h"
#include "mac.h"
#include "alias.h"

// ========== MacArr =========== //

// ===== PRIVATE ===== //
void MacArr::add(const uint8_t* mac) {
    mac_t* a = nullptr;

    // Empty list
    if (empty()) {
        a = &list.data[0];
    }
    // Insert at end
    else if (memcmp(list.data[list.size-1].mac, mac, 6) < 0) {
        a = &list.data[list.size];
    } else {
        // Insert somewhere at start or in beween (insertion sort)
        unsigned int i = 0;

        while (i<list.size && memcmp(list.data[i].mac, mac, 6) < 0) {
            ++i;
        }

        a = &list.data[i];

        // Copy/move everything
        for (int j = list.size; j>i; --j) {
            mac_t* c = &list.data[j];
            mac_t* p = &list.data[j-1];
            memcpy(c->mac, p->mac, 6);
        }
    }

    memcpy(a->mac, mac, 6);
    ++list.size;
}

int MacArr::search(const uint8_t* mac) const {
    // null-pointer or empty list
    if (!mac || (empty())) return -1;

    // Search remaining list
    return bin_search(mac, 0, list.size-1);
}

int MacArr::bin_search(const uint8_t* mac, int low_end, int up_end) const {
    int res;
    int mid = (low_end + up_end) / 2;

    while (low_end <= up_end) {
        res = memcmp(mac, list.data[mid].mac, 6);

        if (res == 0) {
            return mid;
        } else if (res < 0) {
            up_end = mid - 1;
            mid    = (low_end + up_end) / 2;
        } else if (res > 0) {
            low_end = mid + 1;
            mid     = (low_end + up_end) / 2;
        }
    }

    return -1;
}

// ===== PUBLIC ===== //
MacArr::MacArr() {}

MacArr::MacArr(const MacArr& ml) {
    if (ml.list.size > 0) {
        list.data = new mac_t[ml.list.size];

        memcpy(list.data, ml.list.data, sizeof(mac_t)*ml.list.size);
        list.size = ml.list.size;
    }
}

MacArr::MacArr(MacArr&& ml) {
    list = ml.list;

    ml.list.data = nullptr;
    ml.clear();
}

MacArr::MacArr(const String& input, String delimiter) {
    parse(input, delimiter);
}

MacArr::~MacArr() {
    clear();
}

MacArr& MacArr::operator=(const MacArr& ml) {
    if (this != &ml) {
        clear();

        if (ml.list.size > 0) {
            list.data = new mac_t[ml.list.size];

            memcpy(list.data, ml.list.data, sizeof(mac_t)*ml.list.size);
            list.size = ml.list.size;
        }
    }

    return *this;
}

MacArr& MacArr::operator=(MacArr&& ml) {
    if (this != &ml) {
        clear();

        list = ml.list;

        ml.list.data = nullptr;
        ml.clear();
    }

    return *this;
}

void MacArr::clear() {
    if (list.data) {
        delete[] list.data;
        list.data = nullptr;
    }
    list.size = 0;
    list.h    = 0;
}

void MacArr::parse(const String& input, String delimiter) {
    clear();

    SortedStringList str_list(input, delimiter);
    unsigned int     num = 0;

    str_list.begin();

    while (str_list.available()) {
        String mac_str = str_list.iterate();
        if (mac::valid(mac_str.c_str(), mac_str.length()) || (alias::search(mac_str)>=0)) {
            ++num;
        } else {
            str_list.remove();
        }
    }

    if (num == 0) return;

    list.data = new mac_t[num];

    str_list.begin();

    for (unsigned int i = 0; i<num; ++i) {
        String  mac_str = str_list.iterate();
        uint8_t mac[6];

        if (mac::valid(mac_str.c_str(), mac_str.length())) {
            mac::fromStr(mac_str.c_str(), mac);
        } else {
            alias::resolve(mac_str.c_str(), mac);
        }

        add(mac);
    }
}

bool MacArr::contains(const uint8_t* mac) const {
    return search(mac) >= 0;
}

void MacArr::begin() {
    list.h = 0;
}

const uint8_t* MacArr::iterate() {
    if (empty()) return nullptr;
    if (!available()) begin();

    return list.data[list.h++].mac;
}

bool MacArr::available() const {
    return list.h < list.size;
}

int MacArr::size() const {
    return list.size;
}

bool MacArr::empty() const {
    return list.size == 0;
}