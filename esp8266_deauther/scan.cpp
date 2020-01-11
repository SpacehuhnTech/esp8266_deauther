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

    void sniffer(uint8_t* buf, uint16_t len) {
        // drop frames that are too short to have a valid MAC header
        if (len < 28) return;

        // drop deauthentication and disassociation frames
        if ((buf[12] == 0xc0) || (buf[12] == 0xa0)) return;

        // drop beacon and probe response frames
        if ((buf[12] == 0x80) || (buf[12] == 0x50)) return;

        // only allow data frames
        // if(buf[12] != 0x08 && buf[12] != 0x88) return;

        uint8_t* macA = &buf[16]; // To (Receiver)
        uint8_t* macB = &buf[22]; // From (Transmitter)

        // drop frames with corrupted MAC addresses
        if (!is_valid(macA) || !is_valid(macB)) return;

        // drop frames containing mulicast addresses
        if (is_multicast(macA) || is_multicast(macB)) return;

        // probe request to broadcast = unassociated station
        if ((buf[12] == 0x40) && is_broadcast(macA)) {
            debug("Unassociated ");
            debugln(strh::mac(macB));
        }
        // no broadcast address = associated station
        else if (!is_broadcast(macA) && !is_broadcast(macB)) {
            debug("Associated ");
            debug(strh::mac(macA));
            debug(" ");
            debugln(strh::mac(macB));
        }
    }

    // ===== PUBLIC ===== //
    void begin() {
        wifi_set_promiscuous_rx_cb(sniffer);
    }

    void searchAPs() {
        debug("Scanning for access points");

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        WiFi.scanNetworks(true, true);

        int n = WiFi.scanComplete();

        for (int i = 0; i<100 && n < 0; ++i) {
            n = WiFi.scanComplete();
            debug(".");
            delay(200);
        }

        if (n == 0) {
            debugln("No networks found");
        } else {
            debug("Found ");
            debug(n);
            debugln(" networks");

            debug('>');
            debug(' ');
            debug(strh::right(2, "ID"));
            debug(' ');
            debug(strh::left(34, "SSID (Network Name)"));
            debug(' ');
            debug(strh::right(4, "RSSI"));
            debug(' ');
            debug(strh::left(4, "Mode"));
            debug(' ');
            debug(strh::right(2, "Ch"));
            debug(' ');
            debug(strh::left(17, "MAC Address"));
            debug(' ');
            debug(strh::left(8, "Vendor"));
            debugln();

            debugln("===============================================================================");

            for (int i = 0; i < n; ++i) {
                debug(/*__SELECTED__*/ ' ');
                debug(' ');
                debug(strh::right(2, String(i)));
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

            debugln("===============================================================================");
            debugln(">    = Selected");
            debugln("Ch   = Channel (2.4 GHz WiFi)");
            debugln("RSSI = Signal strengh");
            debugln("WPA* = WPA & WPA2 auto mode");
            debugln("===============================================================================");
        }
    }

    void searchSTs() {
        wifi_promiscuous_enable(true);

        debugln("Scanning for stations...");

        for (uint8_t i = 1; i<=14; ++i) {
            debug("On channel ");
            debug(i);
            debugln(":");

            wifi_set_channel(i);
            unsigned long start_time = millis();

            while (millis() - start_time < 3000) {
                delay(1);
            }
            debugln();
        }

        debugln("FINISHED!");

        wifi_promiscuous_enable(false);
    }
}