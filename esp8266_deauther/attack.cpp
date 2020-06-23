/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "attack.h"

#include "debug.h"
#include "cli.h"
#include "strh.h"
#include "sysh.h"
#include "vendor.h"
#include "scan.h"

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
        update_beacon_attack();
        update_deauth_attack();
        update_probe_attack();
    }

    bool active() {
        return beacon_active() || deauth_active() || probe_active();
    }
}