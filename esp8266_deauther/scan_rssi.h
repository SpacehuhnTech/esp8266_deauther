/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

typedef struct rssi_t {
    double        min;
    double        max;
    uint32_t      pkts;
    unsigned long start_time;
} rssi_t;

typedef struct rssi_tmp_t {
    double   rssi;
    uint32_t pkts;
} rssi_tmp_t;

typedef struct rssi_data_t {
    bool enabled;

    // Settings
    rssi_scan_settings_t settings;

    // Temp
    unsigned long last_update_time;
    unsigned long last_ch_update_time;
} rssi_data_t;

rssi_data_t rssi_data;
rssi_tmp_t  rssi_buffer;
rssi_t rssi_stats;

void rssi_sniffer(uint8_t* buf, uint16_t len) {
    SNIFFER_PREAMBLE();

    if (payload_len == 0) return;

    const uint8_t  type     = payload[0];
    const uint8_t* receiver = &payload[4];
    const uint8_t* sender   = &payload[10];
    const int8_t   rssi     = ctrl->rssi;

    if ((rssi_data.settings.macs.size() == 0) || rssi_data.settings.macs.contains(sender)) {
        if (rssi_buffer.pkts == 0) {
            rssi_buffer.pkts = 1;
            rssi_buffer.rssi = rssi;
        } else {
            ++rssi_buffer.pkts;
            rssi_buffer.rssi += ((double)rssi - rssi_buffer.rssi) / (double)rssi_buffer.pkts;
        }
    }
}

void startRSSI(const rssi_scan_settings_t& settings) {
    { // Error checks
        if ((settings.channels & 0x3FFF) == 0) {
            debuglnF("ERROR: No channel specified");
            return;
        }

        if (settings.update_time < 100) {
            debuglnF("ERROR: Update time must be at least 100ms long");
            return;
        }
    }

    scan::stop();

    unsigned long current_time = millis();

    rssi_data.enabled             = true;
    rssi_data.settings            = settings;
    rssi_data.last_update_time    = current_time;
    rssi_data.last_ch_update_time = current_time;

    rssi_stats.min        = -60;
    rssi_stats.max        = -60;
    rssi_stats.pkts       = 0;
    rssi_stats.start_time = current_time;

    rssi_buffer.rssi = -99;
    rssi_buffer.pkts = 0;

    { // Auto correct
        if (sysh::count_ch(rssi_data.settings.channels) <= 1) rssi_data.settings.ch_time = 0;
        else if (rssi_data.settings.ch_time == 0) rssi_data.settings.ch_time = 284;
    }

    { // Output
        debuglnF("[ ===== RSSI Scan ===== ]");

        debugF("Channels:      ");
        debugln(strh::channels(rssi_data.settings.channels));

        debugF("Channel time:  ");
        if (rssi_data.settings.ch_time > 0) debugln(strh::time(rssi_data.settings.ch_time));
        else debuglnF("-");

        debugF("Refresh time:  ");
        debugln(strh::time(rssi_data.settings.update_time));

        debugF("MAC addresses: ");
        debugln(rssi_data.settings.macs.size());

        if (!rssi_data.settings.macs.empty()) {
            debugln();
            debuglnF("MAC");
            debuglnF("=================");

            rssi_data.settings.macs.begin();

            while (rssi_data.settings.macs.available()) {
                debugln(strh::mac(rssi_data.settings.macs.iterate()));
            }

            debuglnF("===================");
        }

        debugln();
        debuglnF("Type 'stop rssi' to stop the scan");
        debugln();

        debuglnF("RSSI      Packets");
        debuglnF("===================");

        sysh::set_next_ch(rssi_data.settings.channels);

        wifi_set_promiscuous_rx_cb(rssi_sniffer);
        wifi_promiscuous_enable(true);
    }
}

void stopRSSI() {
    if (rssi_data.enabled) {
        wifi_promiscuous_enable(false);
        rssi_data.enabled = false;

        rssi_data.settings.macs.clear();

        debuglnF("===================");
        debugln();
        debuglnF("> Stopped RSSI scanner");
        debugln();
    }
}

void update_rssi_scan() {
    if (rssi_data.enabled) {
        unsigned long current_time = millis();

        if (current_time - rssi_data.last_update_time >= rssi_data.settings.update_time) {
            if ((rssi_stats.pkts == 0) || (rssi_buffer.rssi < rssi_stats.min)) rssi_stats.min = rssi_buffer.rssi;
            if ((rssi_stats.pkts == 0) || (rssi_buffer.rssi > rssi_stats.max)) rssi_stats.max = rssi_buffer.rssi;
            rssi_stats.pkts += rssi_buffer.pkts;

            if (rssi_buffer.rssi > -100) debug(' ');
            debug((int)rssi_buffer.rssi);
            debug(' ');

            if (rssi_buffer.rssi > -35) debugF("[======]");
            else if (rssi_buffer.rssi > -45) debugF("[===== ]");
            else if (rssi_buffer.rssi > -55) debugF("[====  ]");
            else if (rssi_buffer.rssi > -65) debugF("[===   ]");
            else if (rssi_buffer.rssi > -75) debugF("[==    ]");
            else if (rssi_buffer.rssi > -85) debugF("[=     ]");
            else debugF("[      ]");

            debug(' ');
            debug(strh::right(5, String((int)rssi_buffer.pkts)));
            debug(' ');
            debuglnF("pkts");

            rssi_buffer.pkts           = 0;
            rssi_data.last_update_time = current_time;
        }

        if ((rssi_data.settings.ch_time > 0) && (current_time - rssi_data.last_ch_update_time >= rssi_data.settings.ch_time)) {
            sysh::set_next_ch(rssi_data.settings.channels);
            rssi_data.last_ch_update_time = current_time;
        }
    }
}

bool rssi_scan_active() {
    return rssi_data.enabled;
}