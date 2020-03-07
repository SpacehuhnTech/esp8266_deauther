#include "eeprom.h"

#include "config.h"

// Used to verify memory
typedef struct boot {
    unsigned int magic_num : 32;
    unsigned int boot_num  : 8;
} boot;

namespace eeprom {
    void begin() {
        EEPROM.begin(EEPROM_SIZE);
    }

    void end() {
        EEPROM.end();
    }

    bool checkBootNum() {
        boot b;

        EEPROM.get(EEPROM_BOOT_ADDR, b);

        if ((b.magic_num == BOOT_MAGIC_NUM) && (b.boot_num < 3)) {
            saveObject(EEPROM_BOOT_ADDR, boot{ BOOT_MAGIC_NUM, ++b.boot_num });
            return true;
        }

        return false;
    }

    void resetBootNum() {
        saveObject(EEPROM_BOOT_ADDR, boot{ BOOT_MAGIC_NUM, 1 });
    }
};