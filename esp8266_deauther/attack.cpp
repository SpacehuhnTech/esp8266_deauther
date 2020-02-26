/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "attack.h"

#include "debug.h"
#include "cli.h"
#include "strh.h"

#include "packetinjector.h"
#include "vendor.h"

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
    #include "attack_probe.h"

    void stop() {
        stopBeacon();
        stopDeauth();
        stopProbe();
    }

    void update() {
        updateBeacon();
        updateDeauth();
        updateProbe();
    }
}