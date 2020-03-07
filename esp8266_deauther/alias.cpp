/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "alias.h"
#include "MACList.h"
#include "strh.h"
#include "mac.h"
#include "debug.h"
#include "eeprom.h"

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
    alias_list_t list;

    void clear() {
        list.size      = 0;
        list.magic_num = ALIAS_MAGIC_NUM;

        add(mac::BROADCAST, "broadcast");

        debugln("Cleared MAC alias list");
    }

    void load() {
        eeprom::getObject(ALIAS_ADDR, list);

        if ((list.magic_num != ALIAS_MAGIC_NUM) || (list.size > MAX_ALIAS_NUM)) {
            debug(list.magic_num);
            debug(" != ");
            debugln(ALIAS_MAGIC_NUM);

            clear();
        } else {
            debugln("Loaded MAC alias list");
        }
    }

    void save() {
        eeprom::saveObject(ALIAS_ADDR, list);
        debugln("Saved MAC alias list");
    }

    bool add(const uint8_t* mac, const String& name) {
        if (list.size < MAX_ALIAS_NUM) {
            alias_t* a = &list.data[list.size];
            memcpy(a->mac, mac, 6);
            strncpy(a->name, name.c_str(), MAX_ALIAS_LEN);
            ++list.size;
            save();
            return true;
        }
        return false;
    }

    String get(const uint8_t* mac) {
        for (unsigned int i = 0; i<list.size; ++i) {
            if (memcmp(list.data[i].mac, mac, 6) == 0) {
                String res;

                for (unsigned int j = 0; j<MAX_ALIAS_LEN; ++j) {
                    res += char(list.data[i].name[j]);
                }

                return res;
            }
        }

        return strh::mac(mac);
    }

    bool resolve(const String& name, uint8_t* buffer) {
        for (unsigned int i = 0; i<list.size; ++i) {
            if (strncmp(list.data[i].name, name.c_str(), MAX_ALIAS_LEN) == 0) {
                memcpy(buffer, list.data[i].mac, 6);
                return true;
            }
        }

        mac::fromStr(name.c_str(), buffer);
        return false;
    }

    void print() {
        debugln("Alias:");

        for (unsigned int i = 0; i<list.size; ++i) {
            debug(i);
            debug(" ");
            debug(strh::mac(list.data[i].mac));
            debug(" ");

            for (unsigned int j = 0; j<MAX_ALIAS_LEN; ++j) {
                debug(list.data[i].name[j]);
            }
            debugln();
        }
    }
}