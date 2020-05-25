/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "alias.h"
#include "strh.h"
#include "mac.h"
#include "debug.h"
#include "eeprom.h"
#include "vendor.h"

typedef struct alias_t {
    uint8_t mac[6];
    char    name[MAX_ALIAS_LEN];
} alias_t;

typedef struct alias_list_t {
    uint32_t magic_num;
    alias_t  data[MAX_ALIAS_NUM];
    int      size;
} alias_list_t;

namespace alias {
    // ===== PRIVATE ===== //
    alias_list_t list;

    int bin_search(const uint8_t* mac, int low_end, int up_end) {
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
    void clear() {
        list.size      = 0;
        list.magic_num = ALIAS_MAGIC_NUM;
        save();
    }

    void load() {
        eeprom::getObject(ALIAS_ADDR, list);

        if ((list.magic_num != ALIAS_MAGIC_NUM) || (list.size > MAX_ALIAS_NUM)) {
            clear();
            // debuglnF("Resetted MAC alias list");
        } else {
            // debuglnF("Loaded MAC alias list");
        }
    }

    void save() {
        eeprom::saveObject(ALIAS_ADDR, list);
    }

    int search(const uint8_t* mac) {
        // null-pointer or empty list
        if (!mac || (list.size == 0)) return -1;

        // Search remaining list
        return bin_search(mac, 0, list.size-1);
    }

    int search(const String& name) {
        for (int i = 0; i<list.size; ++i) {
            if (strncmp(list.data[i].name, name.c_str(), MAX_ALIAS_LEN) == 0) {
                return i;
            }
        }
        return -1;
    }

    bool add(const uint8_t* mac, const String& name) {
        if (name.length() == 0) return false;

        if ((list.size >= MAX_ALIAS_NUM) ||
            (search(mac) > 0) ||
            (search(name) > 0)) return false;

        alias_t* a = nullptr;

        // Empty list
        if (list.size == 0) {
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
                alias_t* c = &list.data[j];
                alias_t* p = &list.data[j-1];
                memcpy(c->mac, p->mac, 6);
                strncpy(c->name, p->name, MAX_ALIAS_LEN);
            }
        }

        memcpy(a->mac, mac, 6);
        strncpy(a->name, name.c_str(), MAX_ALIAS_LEN);
        ++list.size;
        save();

        return true;
    }

    String get(const uint8_t* mac) {
        int id = search(mac);

        if ((id < 0) || (id >= list.size)) return strh::mac(mac);

        return getName(id);
    }

    bool resolve(const String& name, uint8_t* buffer, unsigned int len) {
        int id = search(name);

        if ((id >= 0) && (id < list.size)) {
            memcpy(buffer, list.data[id].mac, len);
            return true;
        } else if (mac::valid(name.c_str(), name.length(), len)) {
            mac::fromStr(name.c_str(), buffer);
            return true;
        }

        return false;
    }

    String getName(int id) {
        if ((id < 0) || (id >= list.size)) return String();

        String res;

        for (unsigned int i = 0; i<MAX_ALIAS_LEN && list.data[id].name[i] != '\0'; ++i) {
            res += char(list.data[id].name[i]);
        }

        return res;
    }

    bool remove(int id) {
        if ((id < 0) || (id >= list.size)) return false;

        for (int i = id; i<list.size; ++i) {
            alias_t* c = &list.data[i];
            alias_t* n = &list.data[i+1];
            memcpy(c->mac, n->mac, 6);
            strncpy(c->name, n->name, MAX_ALIAS_LEN);
        }
        --list.size;
        save();
        return true;
    }

    bool remove(const uint8_t* mac) {
        return remove(search(mac));
    }

    bool remove(const String& name) {
        return remove(search(name));
    }

    void print() {
        debugF("MAC Alias List: ");
        debugln(list.size);
        debuglnF("------------------");

        debuglnF(" ID Name         MAC-Address       Vendor");
        debuglnF("===========================================");

        for (int i = 0; i<list.size; ++i) {
            debug(strh::right(3, String(i)));
            debug(' ');
            debug(strh::left(MAX_ALIAS_LEN, getName(i)));
            debug(' ');
            debug(strh::left(17, strh::mac(list.data[i].mac)));
            debug(' ');
            debug(strh::left(8, vendor::getName(list.data[i].mac)));
            debugln();
        }
        debuglnF("===========================================");

        debugln();
    }
}