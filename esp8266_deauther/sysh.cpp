/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "sysh.h"
// #include "debug.h"

namespace sysh {
    uint8_t count_ch(uint16_t channels) {
        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        return num_of_channels;
    }

    uint8_t next_ch(uint16_t channels) {
        uint8_t ch = wifi_get_channel();

        // If no channels in register
        // Or the only channel that is, is already set
        if (((channels&0x3FFF) == 0) ||
            (((channels >> (ch-1)) & 0x01) && ((channels & ~(1 << (ch-1))) == 0))) return ch;

        do {
            if (++ch > 14) ch = 1;
        } while (!((channels >> (ch-1)) & 0x01));

        // debugF("Get next channel ");
        // debugln(String(ch));

        return ch;
    }
}