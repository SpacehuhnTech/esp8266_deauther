/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "attack.h"

#include "debug.h"
#include "cli.h"
#include "strh.h"

uint8_t probe_pkt[] = {
    /*  0 - 1  */ 0x40, 0x00,                         // Type: Probe Request
    /*  2 - 3  */ 0x00, 0x00,                         // Duration: 0 microseconds
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: Broadcast
    /* 10 - 15 */ 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // Source: random MAC
    /* 16 - 21 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // BSS Id: Broadcast
    /* 22 - 23 */ 0x00, 0x00,                         // Sequence number (handled by the SDK)
    /* 24 - 25 */ 0x00, 0x20,                         // Tag: Set SSID length, Tag length: 32
    /* 26 -    */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /*    - 57 */ 0x00, 0x00, 0x00, 0x00,             // SSID
    /* 58 - 59 */ 0x01, 0x08,                         // Tag Number: Supported Rates (1), Tag length: 8
    /*    60   */ 0x82,                               // 1(B)
    /*    61   */ 0x84,                               // 2(B)
    /*    62   */ 0x8b,                               // 5.5(B)
    /*    63   */ 0x96,                               // 11(B)
    /*    64   */ 0x24,                               // 18
    /*    65   */ 0x30,                               // 24
    /*    66   */ 0x48,                               // 36
    /*    67   */ 0x6c                                // 54
};

/*
   namespace packetinjector {
    bool send_probe(uint8_t ch, uint8_t* from, const char* ssid) {
        memcpy(&probe_pkt[10], from, 6);
        // probe_pkt[25] = strlen(ssid);
        memset(&probe_pkt[26], 0, 32);
        return send(ch, probe_pkt, sizeof(probe_pkt));
    }
   }*/

namespace attack {
    #include "attack_deauth.h"
    #include "attack_beacon.h"

    void update() {
        updateBeacon();
        updateDeauth();
    }
}