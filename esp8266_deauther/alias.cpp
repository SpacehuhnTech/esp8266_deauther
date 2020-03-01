/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "alias.h"
#include "MACList.h"
#include "strh.h"

namespace alias {
    MACList list;

    bool add(const uint8_t* mac, String name) {
        return list.push(mac, name.c_str());
    }

    String get(const uint8_t* mac) {
        MAC* tmp = list.search(mac);

        if (tmp) {
            const char* name = tmp->getName();
            if (name) {
                return String(name);
            }
        }

        return strh::mac(mac);
    }

    const uint8_t* resolve(String name) {
        list.begin();

        MAC* tmp = list.iterate();

        while (list.available()) {
            if (strcmp(tmp->getName(), name.c_str()) == 0) {
                return tmp->getAddr();
            }
        }
        return NULL;
    }
}