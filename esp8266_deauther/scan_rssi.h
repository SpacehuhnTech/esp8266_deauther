/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

void rssi_sniffer(uint8_t* buf, uint16_t len) {
    SNIFFER_PREAMBLE();

    if (payload_len == 0) return;

    const uint8_t  type     = payload[0];
    const uint8_t* receiver = &payload[4];
    const uint8_t* sender   = &payload[10];
    const int8_t   rssi     = ctrl->rssi;

    if ((data.mac_filter.size() == 0) || data.mac_filter.contains(sender)) {
        data.rssi_cb(rssi);
    }
}

void start_rssi_scan() {
    debugF("Scanning for RSSI on ");
    debug(data.num_of_channels);
    debugF(" different channels");
    debugln();
    debuglnF("Type 'stop' to stop the scan");

    uint8_t ch = 1;
    wifi_set_channel(ch);

    if ((data.channels >> (ch-1)) & 0x01) {
        // print_ch(ch);
    } else {
        next_ch();
    }

    wifi_set_promiscuous_rx_cb(rssi_sniffer);
    wifi_promiscuous_enable(true);
}

void stop_rssi_scan() {
    if (data.rssi) {
        wifi_promiscuous_enable(false);
        data.rssi = false;

        data.mac_filter.clear();

        debuglnF("Stopped RSSI scanner");
        debugln();
    }
}

void update_rssi_scan() {
    unsigned long current_time = millis();

    if ((data.ch_time > 0) && (current_time - data.ch_update_time >= data.ch_time)) {
        next_ch();
        data.ch_update_time = current_time;
    }
}