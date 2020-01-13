/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "scan.h"
#include "debug.h"
#include "strh.h"

#include "ESP8266WiFi.h"

extern "C" {
#include "user_interface.h"
#include "scan_types.h"
}

namespace scan {
    // ===== PRIVATE ===== //
    int ap_scan_results = 0;

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

    bool is_valid(uint8_t* mac) {
        for (uint8_t i = 0; i < 6; ++i) {
            if (mac[i] != 0x00) return true;
        }

        return false;
    }

    bool is_multicast(uint8_t* mac) {
        return (mac[0] & 0x01) == 1;
    }

    bool is_broadcast(uint8_t* mac) {
        if (!is_multicast(mac)) return false;

        for (uint8_t i = 0; i < 6; ++i) {
            if (mac[i] != 0xFF) return false;
        }

        return true;
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
        if (!is_valid(mac_a) || !is_valid(mac_b)) return;

        // frame from AP to station
        if (!is_multicast(mac_a)) {
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
    void begin() {}

    void searchAPs() {
        debugln("Scanning for access points (WiFi networks)");

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        station_list_clear(&station_list);
        ap_list_clear(&ap_list);

        WiFi.scanNetworks(true, true);

        ap_scan_results = WiFi.scanComplete();

        for (int i = 0; i<100 && ap_scan_results < 0; ++i) {
            ap_scan_results = WiFi.scanComplete();
            delay(100);
        }

        for (int i = 0; i < ap_scan_results; ++i) {
            ap_t* ap = ap_create(WiFi.SSID(i).c_str(), WiFi.BSSID(i), WiFi.RSSI(i), WiFi.encryptionType(i), WiFi.channel(i));
            ap_list_push(&ap_list, ap);
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

        station_list_clear(&station_list);

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

        for (uint8_t i = 0; i<14; ++i) {
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

                while (millis() - start_time < channel_time) {
                    delay(1);
                }
            }
        }

        wifi_promiscuous_enable(false);

        printSTs();
    }

    void printAPs() {
        if (ap_scan_results == 0) {
            debugln("No access points (networks) found");
        } else {
            debug("Found ");
            debug(ap_scan_results);
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
                debug(strh::left(8, "EXAMPLE3"));
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
                debug(strh::left(8, "EXAMPLE3"));
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
}