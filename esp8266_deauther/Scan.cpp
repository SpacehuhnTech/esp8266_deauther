/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#include "Scan.h"

#include "settings.h"
#include "wifi.h"

Scan::Scan() {
    list = new SimpleList<uint16_t>;
}

void Scan::sniffer(uint8_t* buf, uint16_t len) {
    if (!isSniffing()) return;

    packets++;

    if (len < 28) return;  // drop frames that are too short to have a valid MAC header

    if ((buf[12] == 0xc0) || (buf[12] == 0xa0)) {
        tmpDeauths++;
        return;
    }

    // drop beacon frames, probe requests/responses and deauth/disassociation frames
    if ((buf[12] == 0x80) || (buf[12] == 0x40) || (buf[12] == 0x50) /* || buf[12] == 0xc0 || buf[12] == 0xa0*/) return;

    // only allow data frames
    // if(buf[12] != 0x08 && buf[12] != 0x88) return;

    uint8_t* macTo   = &buf[16];
    uint8_t* macFrom = &buf[22];

    if (macBroadcast(macTo) || macBroadcast(macFrom) || !macValid(macTo) || !macValid(macFrom) || macMulticast(macTo) ||
        macMulticast(macFrom)) return;

    int accesspointNum = findAccesspoint(macFrom);

    if (accesspointNum >= 0) {
        stations.add(macTo, accesspoints.getID(accesspointNum));
    } else {
        accesspointNum = findAccesspoint(macTo);

        if (accesspointNum >= 0) {
            stations.add(macFrom, accesspoints.getID(accesspointNum));
        }
    }
}

int Scan::findAccesspoint(uint8_t* mac) {
    for (int i = 0; i < accesspoints.count(); i++) {
        if (memcmp(accesspoints.getMac(i), mac, 6) == 0) return i;
    }
    return -1;
}

void Scan::start(uint8_t mode) {
    start(mode, sniffTime, scan_continue_mode, continueTime, channelHop, wifi_channel);
}

void Scan::start(uint8_t mode, uint32_t time, uint8_t nextmode, uint32_t continueTime, bool channelHop,
                 uint8_t channel) {
    if (mode != SCAN_MODE_OFF) stop();

    setWifiChannel(channel, true);
    Scan::continueStartTime  = currentTime;
    Scan::snifferPacketTime  = continueStartTime;
    Scan::snifferOutputTime  = continueStartTime;
    Scan::continueTime       = continueTime;
    Scan::sniffTime          = time;
    Scan::channelHop         = channelHop;
    Scan::scanMode           = mode;
    Scan::scan_continue_mode = nextmode;

    if ((sniffTime > 0) && (sniffTime < 1000)) sniffTime = 1000;

    // Serial.printf("mode: %u, time: %u, continue-mode: %u, continueTime: %u, channelHop: %u, channel: %u\r\n", mode,
    // time, scan_continue_mode, continueTime, channelHop, channel);

    /* AP Scan */
    if ((mode == SCAN_MODE_APS) || (mode == SCAN_MODE_ALL)) {
        // remove old results
        accesspoints.removeAll();
        stations.removeAll();
        // start AP scan
        prntln(SC_START_AP);
        WiFi.scanNetworks(true, true);
    }

    /* Station Scan */
    else if (mode == SCAN_MODE_STATIONS) {
        // start station scan
        if (accesspoints.count() < 1) {
            start(SCAN_MODE_ALL);
            // Serial.println(str(SC_ERROR_NO_AP));
            return;
        }
        snifferStartTime = currentTime;
        prnt(SC_START_CLIENT);

        if (sniffTime > 0) prnt(String(sniffTime / 1000) + S);
        else prnt(SC_INFINITELY);

        if (!channelHop) {
            prnt(SC_ON_CHANNEL);
            prnt(wifi_channel);
        }
        prntln();

        // enable sniffer
        wifi::stopAP();
        wifi_promiscuous_enable(true);
    }

    else if (mode == SCAN_MODE_SNIFFER) {
        deauths          = tmpDeauths;
        tmpDeauths       = 0;
        snifferStartTime = currentTime;
        prnt(SS_START_SNIFFER);

        if (sniffTime > 0) prnt(String(sniffTime / 1000) + S);
        else prnt(SC_INFINITELY);
        prnt(SC_ON_CHANNEL);
        prntln(channelHop ? str(SC_ONE_TO) + (String)14 : (String)wifi_channel);

        // enable sniffer
        wifi::stopAP();
        wifi_promiscuous_enable(true);
    }

    /* Stop scan */
    else if (mode == SCAN_MODE_OFF) {
        wifi_promiscuous_enable(false);

        if (settings::getWebSettings().enabled) wifi::resumeAP();
        prntln(SC_STOPPED);
        save(true);

        if (scan_continue_mode != SCAN_MODE_OFF) {
            prnt(SC_RESTART);
            prnt(int(continueTime / 1000));
            prntln(SC_CONTINUE);
        }
    }

    /* ERROR */
    else {
        prnt(SC_ERROR_MODE);
        prntln(mode);
        return;
    }
}

void Scan::update() {
    if (scanMode == SCAN_MODE_OFF) {
        // restart scan if it is continuous
        if (scan_continue_mode != SCAN_MODE_OFF) {
            if (currentTime - continueStartTime > continueTime) start(scan_continue_mode);
        }
        return;
    }

    // sniffer
    if (isSniffing()) {
        // update packet list every 1s
        if (currentTime - snifferPacketTime > 1000) {
            snifferPacketTime = currentTime;
            list->add(packets);

            if (list->size() > SCAN_PACKET_LIST_SIZE) list->remove(0);
            deauths    = tmpDeauths;
            tmpDeauths = 0;
            packets    = 0;
        }

        // print status every 3s
        if (currentTime - snifferOutputTime > 3000) {
            char s[100];

            if (sniffTime > 0) {
                sprintf(s, str(SC_OUTPUT_A).c_str(), getPercentage(), packets, stations.count(), deauths);
            } else {
                sprintf(s, str(SC_OUTPUT_B).c_str(), packets, stations.count(), deauths);
            }
            prnt(String(s));
            snifferOutputTime = currentTime;
        }

        // channel hopping
        if (channelHop && (currentTime - snifferChannelTime > settings::getSnifferSettings().channel_time)) {
            snifferChannelTime = currentTime;

            if (scanMode == SCAN_MODE_STATIONS) nextChannel();  // go to next channel an AP is on
            else setChannel(wifi_channel + 1);                  // go to next channel
        }
    }

    // APs
    if ((scanMode == SCAN_MODE_APS) || (scanMode == SCAN_MODE_ALL)) {
        int16_t results = WiFi.scanComplete();

        if (results >= 0) {
            for (int16_t i = 0; i < results && i < 256; i++) {
                if (channelHop || (WiFi.channel(i) == wifi_channel)) accesspoints.add(i, false);
            }
            accesspoints.sort();
            accesspoints.printAll();

            if (scanMode == SCAN_MODE_ALL) {
                delay(30);
                start(SCAN_MODE_STATIONS);
            }
            else start(SCAN_MODE_OFF);
        }
    }

    // Stations
    else if ((sniffTime > 0) && (currentTime > snifferStartTime + sniffTime)) {
        wifi_promiscuous_enable(false);

        if (scanMode == SCAN_MODE_STATIONS) {
            stations.sort();
            stations.printAll();
        }
        start(SCAN_MODE_OFF);
    }
}

void Scan::setup() {
    save(true);
}

void Scan::stop() {
    scan_continue_mode = SCAN_MODE_OFF;
    start(SCAN_MODE_OFF);
}

void Scan::setChannel(uint8_t ch) {
    if (ch > 14) ch = 1;
    else if (ch < 1) ch = 14;

    wifi_promiscuous_enable(0);
    setWifiChannel(ch, true);
    wifi_promiscuous_enable(1);
}

void Scan::nextChannel() {
    if (accesspoints.count() > 1) {
        uint8_t ch = wifi_channel;

        do {
            ch++;

            if (ch > 14) ch = 1;
        } while (!apWithChannel(ch));
        setChannel(ch);
    }
}

bool Scan::apWithChannel(uint8_t ch) {
    for (int i = 0; i < accesspoints.count(); i++)
        if (accesspoints.getCh(i) == ch) return true;

    return false;
}

void Scan::save(bool force, String filePath) {
    String tmp = FILE_PATH;

    FILE_PATH = filePath;
    save(true);
    FILE_PATH = tmp;
}

void Scan::save(bool force) {
    if (!(accesspoints.changed || stations.changed) && !force) return;

    // Accesspoints
    String buf = String(OPEN_CURLY_BRACKET) + String(DOUBLEQUOTES) + str(SC_JSON_APS) + String(DOUBLEQUOTES) + String(
        DOUBLEPOINT) + String(OPEN_BRACKET); // {"aps":[

    if (!writeFile(FILE_PATH, buf)) {        // overwrite old file
        prnt(F_ERROR_SAVING);
        prntln(FILE_PATH);
        return;
    }

    buf = String(); // clear buffer
    uint32_t apCount = accesspoints.count();

    for (uint32_t i = 0; i < apCount; i++) {
        buf += String(OPEN_BRACKET) + String(DOUBLEQUOTES) + escape(accesspoints.getSSID(i)) + String(DOUBLEQUOTES) +
               String(COMMA);                                                                                    // ["ssid",
        buf += String(DOUBLEQUOTES) + escape(accesspoints.getNameStr(i)) + String(DOUBLEQUOTES) + String(COMMA); // "name",
        buf += String(accesspoints.getCh(i)) + String(COMMA);                                                    // 1,
        buf += String(accesspoints.getRSSI(i)) + String(COMMA);                                                  // -30,
        buf += String(DOUBLEQUOTES) + accesspoints.getEncStr(i) + String(DOUBLEQUOTES) + String(COMMA);          // "wpa2",
        buf += String(DOUBLEQUOTES) + accesspoints.getMacStr(i) + String(DOUBLEQUOTES) + String(COMMA);          // "00:11:22:00:11:22",
        buf += String(DOUBLEQUOTES) + accesspoints.getVendorStr(i) + String(DOUBLEQUOTES) + String(COMMA);       // "vendor",
        buf += b2s(accesspoints.getSelected(i)) + String(CLOSE_BRACKET);                                         // false]

        if (i < apCount - 1) buf += String(COMMA);                                                               // ,

        if (buf.length() >= 1024) {
            if (!appendFile(FILE_PATH, buf)) {
                prnt(F_ERROR_SAVING);
                prntln(FILE_PATH);
                return;
            }

            buf = String(); // clear buffer
        }
    }

    // Stations
    buf += String(CLOSE_BRACKET) + String(COMMA) + String(DOUBLEQUOTES) + str(SC_JSON_STATIONS) + String(DOUBLEQUOTES) +
           String(DOUBLEPOINT) + String(OPEN_BRACKET); // ],"stations":[;
    uint32_t stationCount = stations.count();

    for (uint32_t i = 0; i < stationCount; i++) {
        buf += String(OPEN_BRACKET) + String(DOUBLEQUOTES) + stations.getMacStr(i) + String(DOUBLEQUOTES) +
               String(COMMA);                                                                          // ["00:11:22:00:11:22",
        buf += String(stations.getCh(i)) + String(COMMA);                                              // 1,
        buf += String(DOUBLEQUOTES) + stations.getNameStr(i) + String(DOUBLEQUOTES) + String(COMMA);   // "name",
        buf += String(DOUBLEQUOTES) + stations.getVendorStr(i) + String(DOUBLEQUOTES) + String(COMMA); // "vendor",
        buf += String(*stations.getPkts(i)) + String(COMMA);                                           // 123,
        buf += String(stations.getAP(i)) + String(COMMA);                                              // 0,
        buf += String(DOUBLEQUOTES) + stations.getTimeStr(i) + String(DOUBLEQUOTES) + String(COMMA);   // "<1min",
        buf += b2s(stations.getSelected(i)) + String(CLOSE_BRACKET);                                   // false]

        if (i < stationCount - 1) buf += String(COMMA);                                                // ,

        if (buf.length() >= 1024) {
            if (!appendFile(FILE_PATH, buf)) {
                prnt(F_ERROR_SAVING);
                prntln(FILE_PATH);
                return;
            }

            buf = String(); // clear buffer
        }
    }

    buf += String(CLOSE_BRACKET) + String(CLOSE_CURLY_BRACKET); // ]}

    if (!appendFile(FILE_PATH, buf)) {
        prnt(F_ERROR_SAVING);
        prntln(FILE_PATH);
        return;
    }

    accesspoints.changed = false;
    stations.changed     = false;
    prnt(SC_SAVED_IN);
    prntln(FILE_PATH);
}

uint32_t Scan::countSelected() {
    return accesspoints.selected() + stations.selected() + names.selected();
}

uint32_t Scan::countAll() {
    return accesspoints.count() + stations.count() + names.count();
}

bool Scan::isScanning() {
    return scanMode != SCAN_MODE_OFF;
}

bool Scan::isSniffing() {
    return scanMode == SCAN_MODE_STATIONS || scanMode == SCAN_MODE_SNIFFER;
}

uint8_t Scan::getPercentage() {
    if (!isSniffing()) return 0;

    return (currentTime - snifferStartTime) / (sniffTime / 100);
}

void Scan::selectAll() {
    accesspoints.selectAll();
    stations.selectAll();
    names.selectAll();
}

void Scan::deselectAll() {
    accesspoints.deselectAll();
    stations.deselectAll();
    names.deselectAll();
}

void Scan::printAll() {
    accesspoints.printAll();
    stations.printAll();
    names.printAll();
    ssids.printAll();
}

void Scan::printSelected() {
    accesspoints.printSelected();
    stations.printSelected();
    names.printSelected();
}

uint32_t Scan::getPackets(int i) {
    if (list->size() < SCAN_PACKET_LIST_SIZE) {
        uint8_t translatedNum = SCAN_PACKET_LIST_SIZE - list->size();

        if (i >= translatedNum) return list->get(i - translatedNum);

        return 0;
    } else {
        return list->get(i);
    }
}

String Scan::getMode() {
    switch (scanMode) {
        case SCAN_MODE_OFF:
            return str(SC_MODE_OFF);

        case SCAN_MODE_APS:
            return str(SC_MODE_AP);

        case SCAN_MODE_STATIONS:
            return str(SC_MODE_ST);

        case SCAN_MODE_ALL:
            return str(SC_MODE_ALL);

        case SCAN_MODE_SNIFFER:
            return str(SC_MODE_SNIFFER);

        default:
            return String();
    }
}

double Scan::getScaleFactor(uint8_t height) {
    return (double)height / (double)getMaxPacket();
}

uint32_t Scan::getMaxPacket() {
    uint16_t max = 0;

    for (uint8_t i = 0; i < list->size(); i++) {
        if (list->get(i) > max) max = list->get(i);
    }
    return max;
}

uint32_t Scan::getPacketRate() {
    return list->get(list->size() - 1);
}