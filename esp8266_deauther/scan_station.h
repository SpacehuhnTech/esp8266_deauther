/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// Register a packet from a station
Station* new_pkt(const uint8_t* station_mac, int8_t rssi) {
    // Find station in list
    Station* st = data.st_list.search(station_mac);

    // Not listed yet
    // Push to list
    if (!st && data.st_list.push(station_mac)) {
        // Find station in list
        st = data.st_list.search(station_mac);
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
    ap = data.ap_list.search(receiver);
    if (ap) {
        Station* st = new_pkt(sender, rssi);
        st->setAccessPoint(ap);
        if (st->getPackets() == 1) st->print();
        return;
    }

    // From access point to station
    ap = data.ap_list.search(sender);
    if (ap) {
        Station* st = new_pkt(receiver, rssi);
        st->setAccessPoint(ap);
        if ((st->getPackets() == 1) && !data.silent) st->print();
        return;
    }
}

void new_probe(const uint8_t* sender, const char* ssid, uint8_t len, int rssi) {
    Station* st = new_pkt(sender, rssi);

    if (st) {
        if (st->addProbe(ssid, len) && !data.silent) st->print();
    }
}

void new_auth(const uint8_t* sender, const uint8_t* receiver, int rssi) {
    Station* st = new_pkt(sender, rssi);

    if (st) {
        st->setAccessPoint(nullptr);
        if (st->addAuth(receiver[5]) && !data.silent) st->print();
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

    // authentication
    else if (data.auth && (type == 0xb0) && (memcmp(receiver, data.bssid, 5) == 0)) {
        new_auth(sender, receiver, rssi);
    }
    // anything else that isn't a broadcast frame
    else if (!mac::multicast(receiver)) {
        new_transmission(sender, receiver, rssi);
    }
}

void start_st_scan() {
    debuglnF("[ ===== Station Scan ===== ]");

    debug(strh::left(14, "Scan time:"));
    if (data.timeout > 0) debugln(strh::time(data.timeout));
    else debuglnF("-");

    debug(strh::left(14, "Channel time:"));
    debugln(strh::time(data.ch_time));

    debug(strh::left(14, "Channels:"));

    for (uint8_t i = 0; i<14; ++i) {
        if ((data.channels >> (i)) & 0x01) {
            debug(i+1);
            debug(',');
        }
    }
    debugln();

    debugln();
    debuglnF("Type 'stop' to stop the scan");
    debugln();

    next_ch();

    data.start_time     = millis();
    data.ch_update_time = data.start_time;

    wifi_set_promiscuous_rx_cb(station_sniffer);
    wifi_promiscuous_enable(true);

    if (!data.silent) data.st_list.printHeader();
}

void stop_st_scan() {
    if (data.st) {
        wifi_promiscuous_enable(false);
        data.st = false;

        if (!data.silent) data.st_list.printFooter();

        debuglnF("Stopped station scan");
        debugln();

        printSTs();
    }
}

void update_st_scan() {
    if (!data.ap && data.st) {
        unsigned long current_time = millis();

        if (data.st_list.full()) {
            debuglnF("Station list full");
            stop_st_scan();
        } else if ((data.timeout > 0) && (current_time - data.start_time >= data.timeout)) {
            stop_st_scan();
        } else if ((data.ch_time > 0) && (current_time - data.ch_update_time >= data.ch_time)) {
            next_ch();
            data.ch_update_time = current_time;
        } else if (!data.silent && (current_time - data.output_time >= 1000)) {
            // print infos
            data.output_time = current_time;
        }
    }
}