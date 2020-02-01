/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "scan.h"
#include "debug.h"
#include "strh.h"
#include "vendor.h"
#include "cli.h"
#include "mac.h"

#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

namespace scan {
    // ===== PRIVATE ===== //
    ap_list_t ap_list { NULL, NULL, 0 };
    station_list_t station_list { NULL, NULL, 0 };

    String encstr(int enc) {
        switch (enc) {
            case ENC_TYPE_NONE:
                return "Open";
            case ENC_TYPE_WEP:
                return "WEP";
            case ENC_TYPE_TKIP:
                return "WPA";
            case ENC_TYPE_CCMP:
                return "WPA2";
            case ENC_TYPE_AUTO:
                return "WPA*";
            default:
                return "?";
        }
    }

    void station_sniffer(uint8_t* buf, uint16_t len) {
        // drop frames that are too short to have a valid MAC header
        if (len < 28) return;

        // drop deauthentication and disassociation frames
        if ((buf[12] == 0xc0) || (buf[12] == 0xa0)) return;

        // drop beacon and probe response frames
        if ((buf[12] == 0x80) || (buf[12] == 0x50)) return;

        // only allow data frames
        // if(buf[12] != 0x08 && buf[12] != 0x88) return;

        uint8_t* mac_a = &buf[16]; // To (Receiver)
        uint8_t* mac_b = &buf[22]; // From (Transmitter)

        // drop frames with corrupted MAC addresses
        if (!mac::valid(mac_a) || !mac::valid(mac_b)) return;

        // frame from AP to station
        if (!mac::multicast(mac_a)) {
            ap_t* ap = ap_list_search(&ap_list, mac_b);
            if (ap) {
                if (!station_list_search(&station_list, mac_a, ap)) {
                    station_t* s = station_create(mac_a, ap);

                    debug("Found ");
                    debug(strh::mac(s->mac));
                    debug(" connected to \"");
                    debug(ap->ssid);
                    debugln('"');

                    station_list_push(&station_list, s);
                }
            }
        }
        // broadcast probe request from unassociated station
        else if (buf[12] == 0x40) {
            if (!station_list_search(&station_list, mac_b, NULL)) {
                station_t* s = station_create(mac_b, NULL);

                debug("Found ");
                debug(strh::mac(s->mac));
                debugln();

                station_list_push(&station_list, s);
            }
        }
    }

    // ===== PUBLIC ===== //
    void clearAPresults() {
        station_list_clear(&station_list);
        ap_list_clear(&ap_list);
    }

    void clearSTresults() {
        station_list_clear(&station_list);
    }

    void search(bool ap, bool st, unsigned long time, uint16_t channels, bool retain) {
        { // Error check
            if (!ap && !st) {
                debugln("ERROR: Invalid scan mode");
                return;
            }

            if (st && (time <= 0)) {
                debugln("ERROR: Station scan time equals 0");
                return;
            }

            if (st && (channels == 0)) {
                debugln("ERROR: No channels specified");
                return;
            }
        }

        if (!retain) {
            if (ap) clearAPresults();
            if (st) clearSTresults();
        }

        if (ap) searchAPs();
        if (st) searchSTs(time, channels);
    }

    void searchAPs() {
        debugln("Scanning for access points (WiFi networks)");

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        WiFi.scanNetworks(true, true);

        int n = WiFi.scanComplete();

        for (int i = 0; i<100 && n < 0; ++i) {
            n = WiFi.scanComplete();
            delay(100);
        }

        for (int i = 0; i < n; ++i) {
            if (!ap_list_search(&ap_list, WiFi.BSSID(i))) {
                ap_t* ap = ap_create(WiFi.SSID(i).c_str(), WiFi.BSSID(i), WiFi.RSSI(i), WiFi.encryptionType(i), WiFi.channel(i));
                ap_list_push(&ap_list, ap);
            }
        }

        WiFi.scanDelete();

        printAPs();
    }

    void searchSTs(unsigned long time, uint16_t channels) {
        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        if (num_of_channels == 0) return;

        wifi_set_promiscuous_rx_cb(station_sniffer);
        wifi_promiscuous_enable(true);

        if (time < 1000) time = 1000;
        unsigned long channel_time = time/num_of_channels;

        debug("Scanning for stations (WiFi client devices) on ");
        debug(num_of_channels);
        debug(" different channels");
        debug(" in ");
        debug(time/1000);
        debugln(" seconds");

        bool running = true;

        for (uint8_t i = 0; i<14 && running; ++i) {
            if ((channels >> i) & 0x01) {
                debug("Sniffing on channel ");
                debug(i+1);
                debug(" for ");

                if (channel_time < 1000) {
                    debug(channel_time);
                    debugln(" milliseconds");
                } else {
                    debug(channel_time/1000);
                    debugln(" seconds");
                }

                wifi_set_channel(i+1);
                unsigned long start_time = millis();

                while (running && millis() - start_time < channel_time) {
                    delay(1);
                    running = !cli::read_exit();
                }
            }
        }

        wifi_promiscuous_enable(false);

        printSTs();
    }

    void printAPs() {
        if (ap_list.size == 0) {
            debugln("No access points (networks) found");
        } else {
            debug("Found ");
            debug(ap_list.size);
            debugln(" access points (networks):");

            debug(strh::right(3, "ID"));
            debug(' ');
            debug(strh::left(34, "SSID (Network Name)"));
            debug(' ');
            debug(strh::right(4, "RSSI"));
            debug(' ');
            debug(strh::left(4, "Mode"));
            debug(' ');
            debug(strh::right(2, "Ch"));
            debug(' ');
            debug(strh::left(17, "MAC-Address"));
            debug(' ');
            debug(strh::left(8, "Vendor"));
            debugln();

            debugln("==============================================================================");

            ap_t* h = ap_list.begin;

            for (int i = 0; i<ap_list.size && h; ++i) {
                debug(strh::right(3, String(i)));
                debug(' ');

                if (strlen(h->ssid) > 0) {
                    debug(strh::left(34, '"' + String(h->ssid) + '"'));
                } else {
                    debug(strh::left(34, "*HIDDEN-NETWORK*"));
                }

                debug(' ');
                debug(strh::right(4, String(h->rssi)));
                debug(' ');
                debug(strh::left(4, encstr(h->enc)));
                debug(' ');
                debug(strh::right(2, String(h->ch)));
                debug(' ');
                debug(strh::left(17, strh::mac(h->bssid)));
                debug(' ');
                debug(strh::left(8, vendor::find(h->bssid)));
                debugln();

                h = h->next;
            }

            debugln("================================================================================");
            debugln("Ch   = 2.4 GHz Channel");
            debugln("RSSI = Signal strengh");
            debugln("WPA* = WPA & WPA2 auto mode");
            debugln("================================================================================");
        }
    }

    void printSTs() {
        if (station_list.size == 0) {
            debugln("No stations (clients) found");
        } else {
            debug("Found ");
            debug(station_list.size);
            debugln(" stations (clients):");

            debug(strh::right(3, "ID"));
            debug(' ');
            debug(strh::left(17, "MAC-Address"));
            debug(' ');
            debug(strh::left(34, "Connected to"));
            debug(' ');
            debug(strh::right(4, "Pkts"));
            debug(' ');
            debug(strh::left(8, "Vendor"));
            debugln();

            debugln("======================================================================");

            int i        = 0;
            station_t* h = station_list.begin;

            while (h) {
                debug(strh::right(3, String(i)));
                debug(' ');
                debug(strh::mac(h->mac));
                debug(' ');

                if (h->ap) {
                    debug(strh::left(34, '"' + String(h->ap->ssid) + '"'));
                } else {
                    debug(strh::left(34, "*Unassociated*"));
                }

                debug(' ');
                debug(strh::right(4, String(h->pkts)));
                debug(' ');
                debug(strh::left(8, vendor::find(h->mac)));
                debugln();

                h = h->next;
                ++i;
            }

            debugln("======================================================================");
            debugln("Pkts = Recorded Packets");
            debugln("======================================================================");
        }
    }

    void printResults() {
        printAPs();
        printSTs();
    }

    ap_t* getAP(int id) {
        int   i = 0;
        ap_t* h = ap_list.begin;

        while (h && i<id) {
            h = h->next;
            ++i;
        }

        return h;
    }

    station_t* getStation(int id) {
        int i        = 0;
        station_t* h = station_list.begin;

        while (h && i<id) {
            h = h->next;
            ++i;
        }

        return h;
    }
}