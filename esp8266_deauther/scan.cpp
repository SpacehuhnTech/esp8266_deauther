/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "scan.h"
#include "debug.h"
#include "strh.h"
#include "vendor.h"
#include "cli.h"
#include "mac.h"
#include "attack.h"

#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

#define SNIFFER_PREAMBLE()\
    wifi_pkt_rx_ctrl_t* ctrl { nullptr };\
    uint8_t* payload { nullptr };\
    size_t   payload_len { 0 };\
    if (len == sizeof(wifi_pkt_mgmt_t)) {\
        wifi_pkt_mgmt_t* pkt { (wifi_pkt_mgmt_t*)buf };\
        ctrl        = &pkt->rx_ctrl;\
        payload     = pkt->payload;\
        payload_len = 112;\
    } else if (len == sizeof(wifi_pkt_data_t)) {\
        wifi_pkt_data_t* pkt { (wifi_pkt_data_t*)buf };\
        ctrl        = &pkt->rx_ctrl;\
        payload     = pkt->payload;\
        payload_len = 36;\
    } else if (len == sizeof(wifi_pkt_rx_ctrl_t)) {\
        ctrl = (wifi_pkt_rx_ctrl_t*)buf;\
    }

typedef struct scan_data_t {
    bool          ap;
    bool          st;
    bool          auth;
    bool          rssi;
    uint16_t      channels;
    unsigned long ch_time;
    unsigned long timeout;
    uint8_t       bssid[6];
    bool          silent;
    rssi_cb_f     rssi_cb;

    uint8_t       num_of_channels;
    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkt_time;
    unsigned long ch_update_time;
    unsigned long pkts_received;
    unsigned long pkts_per_second;

    AccessPointList ap_list;
    StationList     st_list;
    MACList         mac_filter;
} scan_data_t;

typedef struct wifi_pkt_rx_ctrl_t {
    signed   rssi          : 8;
    unsigned rate          : 4;
    unsigned is_group      : 1;
    unsigned               : 1;
    unsigned sig_mode      : 2;
    unsigned legacy_length : 12;
    unsigned damatch0      : 1;
    unsigned damatch1      : 1;
    unsigned bssidmatch0   : 1;
    unsigned bssidmatch1   : 1;
    unsigned mcs           : 7;
    unsigned cwb           : 1;
    unsigned ht_length     : 16;
    unsigned smoothing     : 1;
    unsigned not_sounding  : 1;
    unsigned               : 1;
    unsigned aggregation   : 1;
    unsigned stbc          : 2;
    unsigned fec_coding    : 1;
    unsigned sgi           : 1;
    unsigned rx_state      : 8;
    unsigned ampdu_cnt     : 8;
    unsigned channel       : 4;
    unsigned               : 12;
} wifi_pkt_rx_ctrl_t;

typedef struct wifi_pkt_lenseq_t {
    uint16_t length;
    uint16_t seq;
    uint8_t  address3[6];
} wifi_pkt_lenseq_t;

typedef struct wifi_pkt_mgmt_t {
    wifi_pkt_rx_ctrl_t rx_ctrl;
    uint8_t            payload[112];
    uint16_t           cnt;
    uint16_t           len;
} wifi_pkt_mgmt_t;

typedef struct wifi_pkt_data_t {
    wifi_pkt_rx_ctrl_t rx_ctrl;
    uint8_t            payload[36];
    uint16_t           cnt;
    wifi_pkt_lenseq_t  lenseq[1];
} wifi_pkt_data_t;

namespace scan {
    // ===== PRIVATE ===== //
    scan_data_t data;

    void print_ch(uint8_t ch) {
        /*
           if (!data.silent) {
              debugF("Sniff on channel ");
              debug(strh::right(2, String(ch)));
              debugF(" (");
              debug(strh::time(data.ch_time));
              debugln(')');
           }
         */
    }

    void next_ch() {
        if (data.num_of_channels == 0) return;

        uint8_t ch = wifi_get_channel();

        do {
            if (++ch > 14) ch = 1;
        } while (!((data.channels >> (ch-1)) & 0x01));

        print_ch(ch);
        wifi_set_channel(ch);
    }

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

    void clearAPresults() {
        data.ap_list.clear();
    }

    void clearSTresults() {
        data.st_list.clear();
    }

    void startAPsearch() {
        debuglnF("[ ===== Access Point Scan ===== ]");

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        WiFi.scanNetworks(true, true);
    }

    void startSTsearch() {
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

    void stopAPsearch() {
        if (data.ap) {
            WiFi.scanDelete();
            data.ap = false;

            debuglnF("Stopped access point scan");
            debugln();

            printAPs();

            if (data.st) startSTsearch();
        }
    }

    void stopSTsearch() {
        if (data.st) {
            wifi_promiscuous_enable(false);
            data.st = false;

            // if (!data.silent) data.st_list.printFooter();

            debuglnF("Stopped station scan");
            debugln();

            printSTs();
        }
    }

    void stopRSSI() {
        if (data.rssi) {
            wifi_promiscuous_enable(false);
            data.rssi = false;

            data.mac_filter.clear();

            debuglnF("Stopped RSSI scanner");
            debugln();
        }
    }

    void stopAuthSearch() {
        if (data.auth) {
            wifi_promiscuous_enable(false);
            data.auth = false;

            debuglnF("Stopped beacon authentication scan");
            debugln();

            printSTs();
        }
    }

    void updateAPsearch() {
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

            stopAPsearch();
        }
    }

    void updateSTsearch() {
        if (!data.ap && data.st) {
            unsigned long current_time = millis();

            if (data.st_list.full()) {
                debuglnF("Station list full");
                stopSTsearch();
            } else if ((data.timeout > 0) && (current_time - data.start_time >= data.timeout)) {
                stopSTsearch();
            } else if ((data.ch_time > 0) && (current_time - data.ch_update_time >= data.ch_time)) {
                next_ch();
                data.ch_update_time = current_time;
            } else if (!data.silent && (current_time - data.output_time >= 1000)) {
                // print infos
                data.output_time = current_time;
            }
        }
    }

    void updateAuthSearch() {
        if (data.auth && (data.timeout > 0) && (millis() - data.start_time >= data.timeout)) {
            stopAuthSearch();
        }
    }

    void updateRSSI() {
        unsigned long current_time = millis();

        if ((data.ch_time > 0) && (current_time - data.ch_update_time >= data.ch_time)) {
            next_ch();
            data.ch_update_time = current_time;
        }
    }

    // ===== PUBLIC ===== //
    void start(bool ap, bool st, unsigned long timeout, uint16_t channels, unsigned long ch_time, bool silent, bool retain) {
        { // Error check
            if (!ap && !st) {
                debuglnF("ERROR: Invalid scan mode");
                return;
            }

            if (st && (channels == 0)) {
                debuglnF("ERROR: No channels specified");
                return;
            }
        }

        stop();

        if (!retain) {
            if (ap) clearAPresults();
            if (st) clearSTresults();
        }

        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        if (ch_time == 0) ch_time = 284;

        data.ap       = ap;
        data.st       = st;
        data.channels = channels;
        data.ch_time  = ch_time;
        data.timeout  = timeout;
        data.silent   = silent;

        unsigned long current_time = millis();

        data.num_of_channels = num_of_channels;
        data.start_time      = current_time;
        data.output_time     = current_time;
        data.pkt_time        = current_time;
        data.ch_update_time  = current_time;
        data.pkts_received   = 0;
        data.pkts_per_second = 0;

        if (ap) startAPsearch();
        else if (st) startSTsearch();
    }

    void startRSSI(rssi_cb_f rssi_cb, MACList& mac_filter, uint16_t channels, unsigned long ch_time) {
        if (!rssi_cb) {
            // ERROR
            return;
        }

        data.rssi_cb = rssi_cb;
        data.mac_filter.moveFrom(mac_filter);

        /*
                if (data.mac_filter.size() == 0) {
                    // ERROR
                    return;
                }*/

        data.channels = channels;
        data.ch_time  = ch_time;

        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        unsigned long current_time = millis();

        data.rssi            = true;
        data.num_of_channels = num_of_channels;
        data.start_time      = current_time;
        data.output_time     = current_time;
        data.pkt_time        = current_time;
        data.ch_update_time  = current_time;
        data.pkts_received   = 0;
        data.pkts_per_second = 0;

        debugF("Scanning for RSSI on ");
        debug(data.num_of_channels);
        debugF(" different channels");
        debugln();
        debuglnF("Type 'stop' to stop the scan");

        uint8_t ch = 1;
        wifi_set_channel(ch);

        if ((data.channels >> (ch-1)) & 0x01) {
            print_ch(ch);
        } else {
            next_ch();
        }

        wifi_set_promiscuous_rx_cb(rssi_sniffer);
        wifi_promiscuous_enable(true);
    }

    void startAuth(uint8_t* mac, unsigned long timeout, bool silent) {
        stop();

        data.auth       = true;
        data.timeout    = timeout;
        data.silent     = silent;
        data.start_time = millis();
        memcpy(data.bssid, mac, 6);

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

    void stop() {
        stopAPsearch();
        stopSTsearch();
        stopRSSI();
        stopAuthSearch();
    }

    void printAPs(const result_filter_t* filter) {
        data.ap_list.print(filter);
    }

    void printSTs(const result_filter_t* filter) {
        data.st_list.print(filter);
    }

    void update() {
        updateAPsearch();
        updateSTsearch();
        updateAuthSearch();
    }

    AccessPointList& getAccessPoints() {
        return data.ap_list;
    }

    StationList& getStations() {
        return data.st_list;
    }
}