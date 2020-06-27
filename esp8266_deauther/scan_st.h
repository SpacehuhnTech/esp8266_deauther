/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

typedef struct scan_data_t {
    bool enabled;

    // Settings
    st_scan_settings_t settings;

    // Temp
    unsigned long start_time;
    unsigned long ch_update_time;
    unsigned long output_time;
} st_data_t;

st_data_t st_data;

// Register a packet from a station
Station* new_pkt(const uint8_t* station_mac, int8_t rssi) {
    // Find station in list
    Station* st = st_list.search(station_mac);

    // Not listed yet
    // Push to list
    if (!st && st_list.push(station_mac)) {
        // Find station in list
        st = st_list.search(station_mac);
    }

    if (st) {
        st->newPkt(rssi);
    }

    return st;
}

// Register a frame sent from a station to a known AP or vice versa
void new_transmission(const uint8_t* sender, const uint8_t* receiver, int8_t rssi) {
    AccessPoint* ap;

    // From station to access point
    ap = ap_list.search(receiver);
    if (ap) {
        Station* st = new_pkt(sender, rssi);
        st->setAccessPoint(ap);
        // if (st->getPackets() == 1) st->print();
        return;
    }

    // From access point to station
    ap = ap_list.search(sender);
    if (ap) {
        Station* st = new_pkt(receiver, rssi);
        st->setAccessPoint(ap);
        // if (st->getPackets() == 1) st->print();
        return;
    }
}

// Register a new probe frame
void new_probe(const uint8_t* sender, const char* ssid, uint8_t len, int rssi) {
    Station* st = new_pkt(sender, rssi);

    if (st) {
        // if (st->addProbe(ssid, len)) st->print();
        st->addProbe(ssid, len);
    }
}

void station_sniffer(uint8_t* buf, uint16_t len) {
    SNIFFER_PREAMBLE();

    if (payload_len == 0) return;

    uint8_t type = payload[0]; // buf[12];

    // drop ... frames
    if (
        (type == 0xc0) || // deauthentication
        (type == 0xa0) || // disassociation
        (type == 0x80) || // beacon frames
        (type == 0x50)    // probe response
        ) return;

    // only allow data frames
    // if(buf[12] != 0x08 && buf[12] != 0x88) return;

    const uint8_t* receiver = &payload[4];  // &buf[16]; // To (Receiver)
    const uint8_t* sender   = &payload[10]; // &buf[22]; // From (Transmitter)
    const int8_t   rssi     = ctrl->rssi;

    // broadcast probe request
    if ((type == 0x40) && (payload[25] > 0)) {
        uint8_t len      = payload[25];
        const char* ssid = (const char*)&payload[26];

        new_probe(sender, ssid, len, rssi);
    }
    // anything else that isn't a broadcast frame
    else if (!mac::multicast(receiver)) {
        new_transmission(sender, receiver, rssi);
    }
}

void startST(const st_scan_settings_t& settings) {
    { // Error checks
        if ((settings.channels & 0x3FFF) == 0) {
            debuglnF("ERROR: No channel specified");
            return;
        }
    }

    ap::pause();
    scan::stop();

    if (!st_data.settings.retain) st_list.clear();

    unsigned long current_time = millis();

    st_data.enabled        = true;
    st_data.settings       = settings;
    st_data.start_time     = current_time;
    st_data.ch_update_time = current_time;
    st_data.output_time    = current_time;

    { // Auto correct
        if (sysh::count_ch(st_data.settings.channels) <= 1) st_data.settings.ch_time = 0;
        else if (st_data.settings.ch_time == 0) st_data.settings.ch_time = 284;
    }

    { // Output
        debuglnF("[ ===== Scan for Stations ===== ]");

        debugF("Scan time:    ");
        if (st_data.settings.timeout > 0) debugln(strh::time(st_data.settings.timeout));
        else debuglnF("-");

        debugF("Channel time: ");
        if (st_data.settings.ch_time > 0) debugln(strh::time(st_data.settings.ch_time));
        else debuglnF("-");

        debugF("Channels:     ");
        debugln(strh::channels(st_data.settings.channels));

        debugln();
        debuglnF("Type 'stop scan' to stop the scan");
        debugln();

        st_list.printHeader();
    }

    sysh::set_next_ch(st_data.settings.channels);

    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    wifi_set_opmode(STATION_MODE);

    wifi_promiscuous_enable(false);
    wifi_set_promiscuous_rx_cb(station_sniffer);
    wifi_promiscuous_enable(true);
}

void stopST() {
    if (st_data.enabled) {
        wifi_promiscuous_enable(false);
        st_data.enabled = false;

        st_list.printFooter();

        debuglnF("> Stopped station scan");
        debugln();

        print();

        ap::resume();
    }
}

void update_st_scan() {
    if (st_data.enabled) {
        unsigned long current_time = millis();

        if ((st_data.settings.ch_time > 0) && (current_time - st_data.ch_update_time >= st_data.settings.ch_time)) {
            sysh::set_next_ch(st_data.settings.channels);
            st_data.ch_update_time = current_time;
        } else if (current_time - st_data.output_time >= 1000) {
            st_data.output_time = current_time;
            st_list.printBuffer();
        } else if ((st_data.settings.timeout > 0) && (current_time - st_data.start_time >= st_data.settings.timeout)) {
            stopST();
        } else if (st_list.full()) {
            debuglnF("Station list full");
            stopST();
        }
    }
}

bool st_scan_active() {
    return st_data.enabled;
}