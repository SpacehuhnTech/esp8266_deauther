/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#pragma once

// ========== Includes ========== //
#include <EEPROM.h>

// ========== Boot Counter Structure ========== //

// Used for memory verificaiton
#define BOOT_MAGIC_NUM 1234567890

typedef struct boot {
    unsigned int magic_num : 32;
    unsigned int boot_num  : 8;
} boot;

// ========== EEPROM Helper Class ========== //
class EEPROMHelper {
    public:
        static void begin(const int eepromSize) {
            EEPROM.begin(eepromSize);
        }

        static void end() {
            EEPROM.end();
        }

        template<typename T>
        static void saveObject(const int address, const T& t) {
            EEPROM.put(address, t);

            EEPROM.commit();
        }

        template<typename T>
        static void getObject(const int address, const T& t) {
            EEPROM.get(address, t);
        }

        static bool checkBootNum(const int address) {
            boot b;

            EEPROM.get(address, b);

            if ((b.magic_num == BOOT_MAGIC_NUM) && (b.boot_num < 3)) {
                saveObject(address, boot{ BOOT_MAGIC_NUM, ++b.boot_num });
                return true;
            }

            return false;
        }

        static void resetBootNum(const int address) {
            saveObject(address, boot{ BOOT_MAGIC_NUM, 1 });
        }

        static void format(unsigned long size) {
            for (unsigned long i = 0; i<size; i++) EEPROM.write(i, 0x00);
            EEPROM.commit();
        }
};