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
}

namespace scan {
    // ===== PRIVATE ===== //
    typedef struct station_t {
        uint8_t    mac[6];
        int8_t     ap;
        uint32_t   pkts;
        station_t* next;
    } station_t;

    typedef struct station_list_t {
        station_t* begin;
        station_t* end;
        uint8_t    size;
    } station_list_t;

    int ap_scan_results = 0;
    station_list_t station_list { NULL, NULL };

    station_t* station_create(uint8_t* mac, int8_t ap_id) {
        station_t* s = (station_t*)malloc(sizeof(station_t));

        memcpy(s->mac, mac, 6);
        s->ap   = ap_id;
        s->pkts = 1;
        s->next = NULL;

        return s;
    }

    void station_list_push(station_list_t* list, station_t* s) {
        if (list->size < 255) {
            if (!list->begin) {
                list->begin = s;
                list->end   = s;
            } else {
                list->end->next = s;
                list->end       = s;
            }
            ++(list->size);
        }
    }

    bool station_list_search(station_list_t* list, uint8_t* mac, int8_t ap) {
        station_t* h = list->begin;

        while (h) {
            if (memcmp(h->mac, mac, 6) == 0) {
                if ((ap >= 0) && (h->ap < 0)) h->ap = ap;
                ++(h->pkts);
                return true;
            }
            h = h->next;
        }
        return false;
    }

    void station_list_clear(station_list_t* list) {
        station_t* h = list->begin;

        while (h) {
            station_t* to_delete = h;
            h = h->next;
            free(to_delete);
        }

        list->begin = NULL;
        list->end   = NULL;
        list->size  = 0;
    }

    int8_t ap_search(uint8_t* mac) {
        for (int8_t i = 0; i<ap_scan_results && i < 127; ++i) {
            if (memcmp(WiFi.BSSID(i), mac, 6) == 0) {
                return i;
            }
        }
        return -1;
    }

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
            int8_t ap_id = ap_search(mac_b);
            if (ap_id >= 0) {
                if (!station_list_search(&station_list, mac_a, ap_id)) {
                    station_t* s = station_create(mac_a, ap_id);

                    debug("Found ");
                    debug(strh::mac(s->mac));
                    debug(" connected to \"");
                    debug(WiFi.SSID(s->ap));
                    debugln('"');

                    station_list_push(&station_list, s);
                }
            }
        }
        // broadcast probe request from unassociated station
        else if (buf[12] == 0x40) {
            if (!station_list_search(&station_list, mac_b, -1)) {
                station_t* s = station_create(mac_b, -1);

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
        debug("Scanning for access points (WiFi networks)");

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        WiFi.scanNetworks(true, true);

        ap_scan_results = WiFi.scanComplete();

        for (int i = 0; i<100 && ap_scan_results < 0; ++i) {
            ap_scan_results = WiFi.scanComplete();
            debug(".");
            delay(200);
        }

        printAPs();
    }

    void printAPs() {
        if (ap_scan_results == 0) {
            debugln("No networks found");
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

            for (int i = 0; i < ap_scan_results; ++i) {
                debug(strh::right(3, String(i)));
                debug(' ');

                if (WiFi.isHidden(i)) {
                    debug(strh::left(34, "*HIDDEN-NETWORK*"));
                } else {
                    debug(strh::left(34, '"' + WiFi.SSID(i) + '"'));
                }

                debug(' ');
                debug(strh::right(4, String(WiFi.RSSI(i))));
                debug(' ');
                debug(strh::left(4, encstr(WiFi.encryptionType(i))));
                debug(' ');
                debug(strh::right(2, String(WiFi.channel(i))));
                debug(' ');
                debug(strh::left(17, strh::mac(WiFi.BSSID(i))));
                debug(' ');
                debug(strh::left(8, "EXAMPLE3"));
                debugln();
            }

            debugln("================================================================================");
            debugln("Ch   = 2.4 GHz Channel");
            debugln("RSSI = Signal strengh");
            debugln("WPA* = WPA & WPA2 auto mode");
            debugln("================================================================================");
        }
    }

    void searchSTs(unsigned long time) {
        if (time < 1000) time = 1000;
        unsigned long channel_time = time/14;

        station_list_clear(&station_list);

        wifi_set_promiscuous_rx_cb(station_sniffer);
        wifi_promiscuous_enable(true);

        debugln("Scanning for stations (WiFi client devices)");

        for (uint8_t i = 1; i<=14; ++i) {
            debug("Channel ");
            debugln(i);

            wifi_set_channel(i);
            unsigned long start_time = millis();

            while (millis() - start_time < channel_time) {
                delay(1);
            }
        }

        wifi_promiscuous_enable(false);

        printStations();
    }

    void printStations() {
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

            if (h->ap>=0) {
                debug(strh::left(34, '"' + WiFi.SSID(h->ap) + '"'));
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

    void printResults() {
        printAPs();
        printStations();
    }
}