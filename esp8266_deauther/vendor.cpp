/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "vendor.h"
#include "vendor_list.h"

#include "debug.h"
#include "strh.h"

namespace vendor {
    // ===== Private ===== //
    int bin_search(const uint8_t* bytes, int low_end, int up_end) {
        uint8_t list_bytes[3];
        int     res;
        int     mid { (low_end + up_end) / 2 };

        {
            // Check if bigger than list end
            list_bytes[0] = pgm_read_byte_near(vendor_macs + up_end*5);
            list_bytes[1] = pgm_read_byte_near(vendor_macs + up_end*5 + 1);
            list_bytes[2] = pgm_read_byte_near(vendor_macs + up_end*5 + 2);
            res           = memcmp(bytes, list_bytes, 3);
            if (res > 0) return -1;
            if (res == 0) return up_end;

            // Check if smaller than list begin
            list_bytes[0] = pgm_read_byte_near(vendor_macs + low_end*5);
            list_bytes[1] = pgm_read_byte_near(vendor_macs + low_end*5 + 1);
            list_bytes[2] = pgm_read_byte_near(vendor_macs + low_end*5 + 2);
            res           = memcmp(bytes, list_bytes, 3);
            if (res < 0) return -1;
            if (res == 0) return low_end;
        }

        while (low_end <= up_end) {
            list_bytes[0] = pgm_read_byte_near(vendor_macs + mid*5);
            list_bytes[1] = pgm_read_byte_near(vendor_macs + mid*5 + 1);
            list_bytes[2] = pgm_read_byte_near(vendor_macs + mid*5 + 2);

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
    String getName(const uint8_t* mac) {
        if (!mac) return String{};

        int pos_mac = bin_search(mac, 0, sizeof(vendor_macs) / 5 - 1);

        if (pos_mac < 0) return String{};

        int pos_name = pgm_read_byte_near(vendor_macs + pos_mac*5 + 3) |
                       pgm_read_byte_near(vendor_macs + pos_mac*5 + 4) << 8;
        String name;
        char   tmp;

        for (int i = 0; i < 8; ++i) {
            tmp = (char)pgm_read_byte_near(vendor_names + pos_name*8 + i);

            if (tmp != '\0') name += tmp;
            tmp += ' ';
        }

        return name;
    }

    void getRandomMac(uint8_t* mac) {
        randomSeed(os_random());

        int num = random(sizeof(vendor_names) / 11 - 1);
        uint8_t i;

        for (i = 0; i < 3; ++i) mac[i] = pgm_read_byte_near(vendor_macs + num * 5 + i);

        for (i = 3; i < 6; ++i) mac[i] = random(256);
    }

    void getMAC(String name, bool substring, search_cb_f cb) {
        if (!cb) return;

        name.trim();
        name = name.substring(0, 8);
        name.toLowerCase();

        if (name.length() < 2) return;

        int len = sizeof(vendor_macs);

        int pos_name;
        uint8_t prefix[6] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        char    vendor[9];
        vendor[8] = '\0';

        for (int i = 0; i<len; i += 5) {
            pos_name = pgm_read_byte_near(vendor_macs + i + 3) | pgm_read_byte_near(vendor_macs + i + 4) << 8;

            for (int j = 0; j < 8; ++j) {
                vendor[j] = (char)pgm_read_byte_near(vendor_names + pos_name*8 + j);
            }

            String vendor_str { vendor };
            vendor_str.toLowerCase();

            if (substring ? (vendor_str.indexOf(name)>=0) : (name == vendor_str)) {
                for (int j = 0; j < 3; ++j) {
                    prefix[j] = pgm_read_byte_near(vendor_macs + i + j);
                }

                cb(prefix, vendor);
            }
        }
    }
}