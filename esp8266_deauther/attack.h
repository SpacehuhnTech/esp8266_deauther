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

#include "TargetArr.h"

typedef struct beacon_attack_settings_t {
    SortedStringList ssids;
    uint8_t          bssid[6];
    uint8_t          receiver[6];
    int              enc;
    uint16_t         channels;
    uint16_t         pkt_rate;
    unsigned long    timeout;
    bool             scan;
    bool             save_scan;
} beacon_attack_settings_t;

typedef struct deauth_attack_settings_t {
    TargetArr     targets;
    bool          deauth;
    bool          disassoc;
    unsigned long pkt_rate;
    unsigned long timeout;
    unsigned long max_pkts;
} deauth_attack_settings_t;

typedef struct probe_attack_settings_t {
    SortedStringList ssids;
    uint8_t          sender[6];
    uint8_t          receiver[6];
    uint16_t         channels;
    unsigned long    pkt_rate;
    unsigned long    timeout;
} probe_attack_settings_t;

namespace attack {
    void startBeacon(const beacon_attack_settings_t& settings);
    void startDeauth(const deauth_attack_settings_t& settings);
    void startProbe(const probe_attack_settings_t& settings);

    void stopBeacon();
    void stopDeauth();
    void stopProbe();
    void stop();

    void update();

    bool beaconBSSID(uint8_t* bssid);
    String getBeacon(uint8_t num);

    bool active();
}