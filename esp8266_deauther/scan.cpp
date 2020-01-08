/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "scan.h"
#include "debug.h"
#include "strh.h"

#include "ESP8266WiFi.h"

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

    // ===== PUBLIC ===== //
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
}