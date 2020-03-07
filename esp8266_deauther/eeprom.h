#pragma once

#include <EEPROM.h>

namespace eeprom {
    void begin();
    void end();

    bool checkBootNum();
    void resetBootNum();

    template<typename T>
    void saveObject(const int address, const T& t) {
        EEPROM.put(address, t);

        EEPROM.commit();
    }

    template<typename T>
    void getObject(const int address, const T& t) {
        EEPROM.get(address, t);
    }
};