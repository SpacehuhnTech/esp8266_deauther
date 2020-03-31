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
#include "alias.h"

#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

// ========== Sniffer types ========= //
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

// ========== Scan data ========= //
typedef struct auth_buffer_t {
    uint8_t mac[6];
    uint8_t bssid[6];
    int8_t  rssi;
    bool    locked;
} auth_buffer_t;

typedef struct scan_data_t {
    // Modes
    bool ap;
    bool st;
    bool auth;
    bool rssi;

    // Mode Settings
    bool silent;
    bool beacon;

    uint16_t channels;
    uint8_t  num_of_channels;

    unsigned long ch_time;
    unsigned long timeout;

    rssi_cb_f rssi_cb;

    MACList mac_filter;

    // Temp
    unsigned long start_time;
    unsigned long ch_update_time;

    auth_buffer_t auth_buffer;

    // Results
    AccessPointList ap_list;
    StationList     st_list;
} scan_data_t;

namespace scan {
    // ===== PRIVATE ===== //
    scan_data_t data;

    uint8_t count_channels(uint16_t channels) {
        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        return num_of_channels;
    }

    void next_ch() {
        if (data.num_of_channels == 0) return;

        uint8_t ch = wifi_get_channel();

        do {
            if (++ch > 14) ch = 1;
        } while (!((data.channels >> (ch-1)) & 0x01));

        /*
           if (!data.silent) {
              debugF("Sniff on channel ");
              debug(strh::right(2, String(ch)));
              debugF(" (");
              debug(strh::time(data.ch_time));
              debugln(')');
           }
         */

        wifi_set_channel(ch);
    }

#include "scan_station.h"
#include "scan_ap.h"
#include "scan_rssi.h"
#include "scan_auth.h"

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
            if (ap) data.ap_list.clear();
            if (st) data.st_list.clear();
        }

        unsigned long current_time = millis();

        data.ap              = ap;
        data.st              = st;
        data.silent          = silent;
        data.channels        = channels;
        data.num_of_channels = count_channels(channels);
        data.ch_time         = ch_time;
        data.timeout         = timeout;
        data.start_time      = current_time;
        data.ch_update_time  = current_time;

        if (data.num_of_channels == 0) data.ch_time = 0;
        if ((data.ch_time == 0) && (data.num_of_channels > 1)) data.ch_time = 284;

        if (ap) start_ap_scan();
        else if (st) start_st_scan();
    }

    void startAuth(bool beacon, MACList* receivers, unsigned long timeout, uint16_t channels, unsigned long ch_time) {
        stop();

        unsigned long current_time = millis();

        data.auth            = true;
        data.beacon          = beacon;
        data.channels        = channels;
        data.num_of_channels = count_channels(channels);
        data.ch_time         = ch_time;
        data.timeout         = timeout;
        data.start_time      = current_time;
        data.ch_update_time  = current_time;

        if (data.num_of_channels == 0) data.ch_time = 0;
        if ((data.num_of_channels > 1) && (data.ch_time == 0)) data.ch_time = 284;

        if (receivers) data.mac_filter.moveFrom(*receivers);

        start_auth_scan();
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
        data.ch_update_time  = current_time;

        start_rssi_scan();
    }

    void stop() {
        stop_ap_scan();
        stop_st_scan();
        stop_rssi_scan();
        stop_auth_scan();
        data.mac_filter.clear();
    }

    void printAPs(const result_filter_t* filter) {
        data.ap_list.print(filter);
    }

    void printSTs(const result_filter_t* filter) {
        data.st_list.print(filter);
    }

    void update() {
        update_ap_scan();
        update_st_scan();
        update_auth_scan();
        update_rssi_scan();
    }

    AccessPointList& getAccessPoints() {
        return data.ap_list;
    }

    StationList& getStations() {
        return data.st_list;
    }
}