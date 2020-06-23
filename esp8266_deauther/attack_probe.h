/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// ========== PROBE PACKET ========== //
const uint8_t probe_pkt[] = {
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
    bool enabled;

    probe_attack_settings_t settings;

    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkts_sent;
    unsigned long pkts_per_second;
    unsigned long pkt_time;
    unsigned long pkt_interval;
} probe_attack_data_t;

probe_attack_data_t probe_data;

// ========== SEND FUNCTION ========== //
bool send_probe(uint8_t ch, uint8_t* sender, uint8_t* receiver, const char* ssid) {
    size_t ssid_len = strlen(ssid);

    if (ssid_len > 32) ssid_len = 32;
    uint16_t pkt_len = 0;

    uint8_t frame[68];

    // MAC header
    memcpy(&frame[0], &probe_pkt[0], 4);
    memcpy(&frame[4], receiver, 6);
    memcpy(&frame[10], sender, 6);
    memcpy(&frame[16], receiver, 6);
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

    return sysh::send(ch, frame, pkt_len);
}

// ========== ATTACK FUNCTIONS ========== //
void startProbe(const probe_attack_settings_t& settings) {
    { // Error checks
        if (settings.ssids.empty()) {
            debuglnF("ERROR: No SSIDs specified");
            return;
        }
        if ((settings.channels & 0x3FFF) == 0) {
            debuglnF("ERROR: No channels specified");
            return;
        }
    }

    stopProbe();

    unsigned long current_time = millis();

    probe_data.enabled  = true;
    probe_data.settings = settings;
    if (probe_data.settings.pkt_rate == 0) probe_data.settings.pkt_rate = 1;
    probe_data.start_time      = current_time;
    probe_data.output_time     = current_time;
    probe_data.pkts_sent       = 0;
    probe_data.pkts_per_second = 0;
    probe_data.pkt_time        = current_time;
    probe_data.pkt_interval    = 1000/probe_data.settings.pkt_rate;

    { // Output
        debuglnF("[ ===== Probe Attack ===== ]");
        debugF("Sender:             ");
        debugln(strh::mac(probe_data.settings.sender));

        debugF("Receiver:           ");
        debugln(strh::mac(probe_data.settings.receiver));

        debugF("Channels:           ");
        debugln(strh::channels(probe_data.settings.channels));

        debugF("Packets/s per SSID: ");
        debugln(probe_data.settings.pkt_rate);

        debugF("Timeout:            ");
        if (probe_data.settings.timeout > 0) debugln(strh::time(probe_data.settings.timeout));
        else debuglnF("-");

        debugF("SSID:               ");
        debugln(probe_data.settings.ssids.size());

        // Print SSID List
        debugln();
        debuglnF("SSID");
        debuglnF("==================================");

        probe_data.settings.ssids.begin();

        while (probe_data.settings.ssids.available()) {
            debugln(strh::left(32, '"'+probe_data.settings.ssids.iterate()+'"'));
        }
        debuglnF("==================================");

        debugln();
        debuglnF("Type 'stop probe' to stop the attack");
        debugln();
    }
}

void stopProbe() {
    if (probe_data.enabled) {
        probe_data.enabled    = false;
        probe_data.pkts_sent += probe_data.pkts_per_second;
        probe_data.settings.ssids.clear();

        debugF("> Stopped probe attack. Sent ");
        debug(probe_data.pkts_sent);
        debuglnF(" packets.");
    }
}

void update_probe_attack() {
    if (probe_data.enabled) {
        if (((probe_data.settings.timeout > 0) && (millis() - probe_data.start_time > probe_data.settings.timeout))) {
            stopProbe();
            return;
        }

        if (millis() - probe_data.pkt_time >= probe_data.pkt_interval) {
            probe_data.pkt_time = millis();
            probe_data.settings.ssids.begin();

            uint8_t ch = sysh::next_ch(probe_data.settings.channels);

            for (int i = 0; i<probe_data.settings.ssids.size(); ++i) {
                String ssid = probe_data.settings.ssids.iterate();

                probe_data.pkts_per_second += send_probe(ch,
                                                         probe_data.settings.sender,
                                                         probe_data.settings.receiver,
                                                         ssid.c_str());

                delay(1);
            }
        }

        if (millis() - probe_data.output_time >= 1000) {
            probe_data.pkts_sent += probe_data.pkts_per_second;

            /*
                        debugF("[Probe attack: ");
                        debug(probe_data.pkts_per_second);
                        debugF(" pkts/s, ");
                        debug(probe_data.pkts_sent);
                        debuglnF(" total]");
             */
            probe_data.output_time = millis();

            probe_data.pkts_per_second = 0;
        }
    }
}

bool probe_active() {
    return probe_data.enabled;
}