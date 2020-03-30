/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

void start_auth_scan() {
    debugF("Scanning for authentications on ");
    debugln(strh::mac(data.bssid));
    debugln();

    // Reset authentications
    data.st_list.begin();

    while (data.st_list.available()) {
        Station* tmp = data.st_list.iterate();
        tmp->setAuth(0);
    }

    wifi_set_promiscuous_rx_cb(station_sniffer);
    wifi_promiscuous_enable(true);
}

void stop_auth_scan() {
    if (data.auth) {
        wifi_promiscuous_enable(false);
        data.auth = false;

        debuglnF("Stopped beacon authentication scan");
        debugln();

        printSTs();
    }
}

void update_auth_scan() {
    if (data.auth && (data.timeout > 0) && (millis() - data.start_time >= data.timeout)) {
        stop_auth_scan();
    }
}