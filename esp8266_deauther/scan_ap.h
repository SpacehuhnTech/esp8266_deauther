/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

void start_ap_scan() {
    debuglnF("[ ===== Access Point Scan ===== ]");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    WiFi.scanNetworks(true, true);
}

void stop_ap_scan() {
    if (data.ap) {
        WiFi.scanDelete();
        data.ap = false;

        debuglnF("Stopped access point scan");
        debugln();

        printAPs();

        if (data.st) start_st_scan();
    }
}

void update_ap_scan() {
    if (data.ap && (WiFi.scanComplete() >= 0)) {
        int n = WiFi.scanComplete();

        for (int i = 0; i < n; ++i) {
            if (((data.channels >> (WiFi.channel(i)-1)) & 0x01)) {
                data.ap_list.push(
                    WiFi.isHidden(i),
                    WiFi.SSID(i).c_str(),
                    WiFi.BSSID(i),
                    WiFi.RSSI(i),
                    WiFi.encryptionType(i),
                    WiFi.channel(i)
                    );
            }
        }

        stop_ap_scan();
    }
}