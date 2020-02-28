/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "StringList.h"

#define ENCRYPTION_OPEN 0
// #define ENCRYPTION_WEP 1
// #define ENCRYPTION_WPA 2
#define ENCRYPTION_WPA2 3
// #define ENCRYPTION_WPA_AUTO 4
// #define ENCRYPTION_WPA3 5

#include "TargetList.h"

namespace attack {
    void startBeacon(StringList& ssid_list, uint8_t* from, uint8_t* to, int enc, uint8_t ch, unsigned long timeout, bool silent);
    void startDeauth(TargetList& targets, bool deauth, bool disassoc, unsigned long rate, unsigned long timeout, unsigned long pkts, bool silent);
    void startProbe(StringList& ssid_list, uint8_t* to, uint8_t ch, unsigned long timeout, bool silent);

    void stopBeacon();
    void stopDeauth();
    void stopProbe();
    void stop();

    void updateBeacon();
    void updateDeauth();
    void updateProbe();
    void update();
}