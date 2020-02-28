/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "vendor.h"
#include "vendor_list.h"

namespace vendor {
    // ===== Private ===== //
    int bin_search(const uint8_t* bytes, int low_end, int up_end) {
        uint8_t list_bytes[3];
        int     res;
        int     mid = (low_end + up_end) / 2;

        while (low_end <= up_end) {
            list_bytes[0] = pgm_read_byte_near(vendor_macs + mid * 5);
            list_bytes[1] = pgm_read_byte_near(vendor_macs + mid * 5 + 1);
            list_bytes[2] = pgm_read_byte_near(vendor_macs + mid * 5 + 2);

            res = memcmp(bytes, list_bytes, 3);

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

    // ===== Public ===== //
    String search(const uint8_t* mac) {
        String name;
        int    pos_mac  = bin_search(mac, 0, sizeof(vendor_macs) / 5 - 1);
        int    pos_name = pgm_read_byte_near(vendor_macs + pos_mac * 5 + 3) | pgm_read_byte_near(vendor_macs + pos_mac * 5 + 4) << 8;

        if (pos_mac >= 0) {
            char tmp;

            for (int i = 0; i < 8; i++) {
                tmp = (char)pgm_read_byte_near(vendor_names + pos_name * 8 + i);

                if (tmp != '\0') name += tmp;
                tmp += ' ';
            }
        }

        return name;
    }

    void randomize(uint8_t* mac) {
        randomSeed(os_random());

        int num = random(sizeof(vendor_names) / 11 - 1);
        uint8_t i;

        for (i = 0; i < 3; i++) mac[i] = pgm_read_byte_near(vendor_macs + num * 5 + i);

        for (i = 3; i < 6; i++) mac[i] = random(256);
    }
}