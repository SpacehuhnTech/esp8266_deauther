/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

typedef struct ap_data_t {
    bool               enabled;
    ap_scan_settings_t settings;
} ap_data_t;

ap_data_t ap_data;

void startAP(const ap_scan_settings_t& settings) {
    { // Error check
        if ((settings.channels & 0x3FFF) == 0) {
            debuglnF("ERROR: No channel specified");
            return;
        }
    }

    scan::stop();

    if (!settings.retain) ap_list.clear();

    ap_data.enabled  = true;
    ap_data.settings = settings;

    { // Auto correct
    }

    { // Output
        debuglnF("[ ===== Access Point Scan ===== ]");
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    WiFi.scanNetworks(true, true);
}

void stopAP() {
    if (ap_data.enabled) {
        WiFi.scanDelete();
        ap_data.enabled = false;

        debuglnF("Stopped access point scan");
        debugln();

        printAPs();

        if (ap_data.settings.st) startST(ap_data.settings.st_settings);
    }
}

void update_ap_scan() {
    if (ap_data.enabled && (WiFi.scanComplete() >= 0)) {
        int n = WiFi.scanComplete();

        for (int i = 0; i < n; ++i) {
            if (((ap_data.settings.channels >> (WiFi.channel(i)-1)) & 0x01)) {
                ap_list.push(
                    WiFi.isHidden(i),
                    WiFi.SSID(i).c_str(),
                    WiFi.BSSID(i),
                    WiFi.RSSI(i),
                    WiFi.encryptionType(i),
                    WiFi.channel(i)
                    );
            }
        }

        stopAP();
    }
}