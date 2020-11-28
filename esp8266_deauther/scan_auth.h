/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

typedef struct auth_buffer_t {
    uint8_t ch;
    uint8_t mac[6];
    uint8_t bssid[6];
    int8_t  rssi;
    bool    locked;
} auth_buffer_t;

typedef struct auth_data_t {
    bool enabled;

    // Settings
    auth_scan_settings_t settings;

    // Temp
    unsigned long start_time;
    unsigned long ch_update_time;
} auth_data_t;

auth_data_t   auth_data;
auth_buffer_t auth_buffer;

void auth_sniffer(uint8_t* buf, uint16_t len) {
    if (auth_buffer.locked) return;

    SNIFFER_PREAMBLE();

    if (payload_len == 0) return;

    const uint8_t  type     = payload[0];   // buf[12];
    const uint8_t* receiver = &payload[4];  // &buf[16]; // To (Receiver)
    const uint8_t* sender   = &payload[10]; // &buf[22]; // From (Transmitter)
    const int8_t   rssi     = ctrl->rssi;

    if (auth_data.settings.save && (type == 0x40) && (payload[25] > 0)) {
        // broadcast probe request
        uint8_t len      = payload[25];
        const char* ssid = (const char*)&payload[26];

        Station* st = new_pkt(sender, rssi);
        if (st) st->addProbe(ssid, len);
    }

    // drop everything that isn't an authentication frame
    if (type != 0xb0) return;

    const uint8_t ch = wifi_get_channel(); // ctrl->channel;

    // Drop packets that aren't in the BSSID filter (if filter provided)
    if (!auth_data.settings.bssids.empty() &&
        !auth_data.settings.bssids.contains(receiver))
        return;

    auth_buffer.rssi = rssi;
    auth_buffer.ch   = ch;
    memcpy(auth_buffer.mac, sender, 6);
    memcpy(auth_buffer.bssid, receiver, 6);
    auth_buffer.locked = true;
}

void startAuth(const auth_scan_settings_t& settings) {
    { // Error checks
        if ((settings.channels & 0x3FFF) == 0) {
            debuglnF("ERROR: No channel specified");
            return;
        }
    }

    scan::stop();

    unsigned long current_time = millis();

    auth_data.enabled        = true;
    auth_data.settings       = settings;
    auth_data.start_time     = current_time;
    auth_data.ch_update_time = current_time;

    auth_buffer.locked = false;

    { // Auto correct
        if (sysh::count_ch(auth_data.settings.channels) <= 1) auth_data.settings.ch_time = 0;
        else if (auth_data.settings.ch_time == 0) auth_data.settings.ch_time = 284;
    }

    { // Output
        debuglnF("[ ===== Authentication Scan ===== ]");

        debugF("Scan time:     ");
        if (auth_data.settings.timeout > 0) debugln(strh::time(auth_data.settings.timeout));
        else debuglnF("-");

        debugF("Channels:      ");
        debugln(strh::channels(auth_data.settings.channels));

        debugF("Channel time:  ");
        if (auth_data.settings.ch_time > 0) debugln(strh::time(auth_data.settings.ch_time));
        else debuglnF("-");

        debugF("Beacon Mode:   ");
        debugln(auth_data.settings.beacon ? F("On") : F("Off"));

        debugF("Save stations: ");
        debugln(auth_data.settings.save ? F("Yes") : F("No"));

        debugF("BSSID filter:  ");
        debugln(auth_data.settings.bssids.size());

        if (!auth_data.settings.bssids.empty()) {
            debugln();
            debuglnF("BSSID");
            debuglnF("=================");

            auth_data.settings.bssids.begin();

            while (auth_data.settings.bssids.available()) {
                debugln(strh::mac(auth_data.settings.bssids.iterate()));
            }

            debuglnF("=================");
        }

        debugln();
        debuglnF("Type 'stop auth' to stop the scan");
        debugln();

        debuglnF("RSSI Ch Vendor   MAC-Address       SSID                               BSSID");
        debuglnF("=======================================================================================");
    }

    if (!auth_data.settings.beacon) sysh::set_next_ch(auth_data.settings.channels);

    wifi_set_promiscuous_rx_cb(auth_sniffer);
    wifi_promiscuous_enable(true);
}

void stopAuth() {
    if (auth_data.enabled) {
        wifi_promiscuous_enable(false);
        auth_data.enabled = false;

        auth_data.settings.bssids.clear();

        debuglnF("=======================================================================================");
        debuglnF("Ch = 2.4 GHz Channel    ,    RSSI = Signal strength    ,    BSSID = Network MAC address");
        debuglnF("=======================================================================================");
        debugln();
        debuglnF("> Stopped authentication scan");
        debugln();
    }
}

void update_auth_scan() {
    if (auth_data.enabled) {
        unsigned long current_time = millis();

        // Print scan results if something is in the buffer
        if (auth_buffer.locked) {
            // Copy data and unlock the buffer
            auth_buffer_t tmp = auth_buffer;
            auth_buffer.locked = false;

            // Don't print unrelated auths when attacking
            if (auth_data.settings.beacon && !attack::beaconBSSID(tmp.bssid)) return;

            debug(strh::right(4, String(tmp.rssi)));
            debug(' ');
            debug(strh::right(2, String(tmp.ch)));
            debug(' ');
            debug(strh::left(8, vendor::getName(tmp.mac)));
            debug(' ');
            debug(strh::left(17, alias::get(tmp.mac)));
            debug(' ');

            // Part of beacon attack
            if (attack::beaconBSSID(tmp.bssid)) {
                debug(strh::left(34, '"'+attack::getBeacon(tmp.bssid[5])+'"'));
                debug(' ');
                debugln(strh::left(17, alias::get(tmp.bssid)));
                return;
            }

            // Part of AP list
            AccessPoint* ap = ap_list.search(tmp.bssid);
            if (ap) {
                debug(strh::left(34, ap->getSSIDString()));
                debug(' ');
                debugln(strh::left(17, ap->getBSSIDString()));
                return;
            }

            // Unknown AP
            debug(strh::left(34, "*UNKNOWN*"));
            debug(' ');
            debugln(strh::left(17, alias::get(tmp.bssid)));
        }

        if ((auth_data.settings.ch_time > 0) && (!auth_data.settings.beacon) && (current_time - auth_data.ch_update_time >= auth_data.settings.ch_time)) {
            sysh::set_next_ch(auth_data.settings.channels);
            auth_data.ch_update_time = current_time;
        } else if ((auth_data.settings.timeout > 0) && (millis() - auth_data.start_time >= auth_data.settings.timeout)) {
            stopAuth();
        }
    }
}

bool auth_scan_active() {
    return auth_data.enabled;
}