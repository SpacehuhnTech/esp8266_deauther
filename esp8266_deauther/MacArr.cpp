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

#include "config.h"

#ifdef DEBUG_MAC_ARR
#include "debug.h"
#include "strh.h"
#else // ifdef DEBUG_MAC_ARR
#define debug(...) 0
#define debugln(...) 0
#define debugf(...) 0
#define debugF(...) 0
#define debuglnF(...) 0
#endif // ifdef DEBUG_MAC_ARR

// ========== MacArr =========== //

// ===== PRIVATE ===== //

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

MacArr::MacArr(int size) {
    if (size > 0) {
        debuglnF("[MacArr] size constructor");
        list.data = new mac_t[size];
        list.max  = size;
        list.h    = 0;
    }
}

MacArr::MacArr(const MacArr& ml) {
    if (ml.list.size > 0) {
        debuglnF("[MacArr] copy constructor");

        list.data = new mac_t[ml.list.size];

        memcpy(list.data, ml.list.data, sizeof(mac_t)*ml.list.size);
        list.size = ml.list.size;
        list.max  = ml.list.size;
    }
}

MacArr::MacArr(MacArr&& ml) {
    debuglnF("[MacArr] move constructor");

    list = ml.list;

    ml.list.data = nullptr;
    ml.clear();
}

MacArr::MacArr(const String& input, String delimiter) {
    debuglnF("[MacArr] parse constructor");
    parse(input, delimiter);
}

MacArr::~MacArr() {
    debuglnF("[MacArr] destructor");
    clear();
}

MacArr& MacArr::operator=(const MacArr& ml) {
    debuglnF("[MacArr] copy assignment");

    if (this != &ml) {
        clear();

        if (ml.list.size > 0) {
            list.data = new mac_t[ml.list.size];

            memcpy(list.data, ml.list.data, sizeof(mac_t)*ml.list.size);
            list.size = ml.list.size;
            list.max  = ml.list.size;
        }
    }

    return *this;
}

MacArr& MacArr::operator=(MacArr&& ml) {
    debuglnF("[MacArr] move assignment");

    if (this != &ml) {
        clear();

        list = ml.list;

        ml.list.data = nullptr;
        ml.clear();
    }

    return *this;
}

MacArr& MacArr::operator+=(const MacArr& ml) {
    debuglnF("[MacArr] += operator begin");

    if ((this != &ml) && (ml.list.size > 0)) {
        // New list without data and max set to current size
        mac_list_t new_list { nullptr, list.size, 0, 0 };

        // Count new data
        for (unsigned int i = 0; i<ml.list.size; ++i) {
            if (!contains(ml.list.data[i].mac)) {
                ++new_list.max;
            }
        }

        // Create new buffer
        new_list.data = new mac_t[new_list.max];

        // Copy current data
        memcpy(new_list.data, list.data, sizeof(mac_t)*list.size);
        new_list.size = list.size;

        // Delete current data
        clear();

        // Replace list with new list
        list = new_list;

        // Copy new data
        for (unsigned int i = 0; i<ml.list.size; ++i) {
            add(ml.list.data[i].mac);
        }
    }

    debuglnF("[MacArr] += operator end");

    return *this;
}

void MacArr::clear() {
    debuglnF("[MacArr] clear");

    if (list.data) {
        delete[] list.data;
        list.data = nullptr;
    }
    list.size = 0;
    list.h    = 0;
}

void MacArr::parse(const String& input, String delimiter) {
    debuglnF("[MacArr] parse");
    clear();

    SortedStringList str_list { input, delimiter };
    unsigned int     num { 0 };

    str_list.begin();

    while (str_list.available()) {
        String mac_str { str_list.iterate() };
        if (mac::valid(mac_str.c_str(), mac_str.length()) || (alias::search(mac_str)>=0)) {
            ++num;
        } else {
            debugF("[MacArr] invalid MAC string \"");
            debug(mac_str);
            debuglnF("\"");
            str_list.remove();
        }
    }

    if (num == 0) return;

    list.data = new mac_t[num];
    list.max  = num;

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

bool MacArr::add(const uint8_t* mac) {
    debugF("[MacArr] add...");

    if (full()) {
        debuglnF("full");
        return false;
    }

    mac_t* a = nullptr;

    // Empty list
    if (empty()) {
        a = &list.data[0];
    }
    // Insert at end
    else if (memcmp(list.data[list.size-1].mac, mac, 6) < 0) {
        a = &list.data[list.size];
    }
    // Insert somewhere at start or in beween (insertion sort)
    else {
        unsigned int i = 0;
        int res        = memcmp(list.data[i].mac, mac, 6);

        while (i<list.size && res < 0) {
            ++i;
            res = memcmp(list.data[i].mac, mac, 6);
        }

        // Ignore duplicates
        if (res == 0) {
            debuglnF("duplicate");
            return false;
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

    debuglnF("OK");
    return true;
}

bool MacArr::contains(const uint8_t* mac) const {
    return search(mac) >= 0;
}

void MacArr::begin() {
    debuglnF("[MacArr] begin");
    list.h = 0;
}

const uint8_t* MacArr::iterate() {
    debuglnF("[MacArr] iterate");

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

bool MacArr::full() const {
    return list.size >= list.max;
}

bool MacArr::empty() const {
    return list.size == 0;
}