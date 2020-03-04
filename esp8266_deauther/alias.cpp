/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "alias.h"
#include "MACList.h"
#include "strh.h"
#include "mac.h"

namespace alias {
    MACList list;

    bool add(const uint8_t* mac, const String& name) {
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

    bool resolve(const String& name, uint8_t* buffer) {
        list.begin();

        MAC* tmp = list.iterate();

        while (list.available()) {
            if (strcmp(tmp->getName(), name.c_str()) == 0) {
                memcpy(buffer, tmp->getAddr(), 6);
                return true;
            }
        }

        mac::fromStr(name.c_str(), buffer);
        return false;
    }
}