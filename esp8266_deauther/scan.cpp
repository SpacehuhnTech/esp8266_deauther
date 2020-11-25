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
#include "sysh.h"
#include "ap.h"

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

namespace scan {
    // ===== PRIVATE ===== //
    AccessPointList ap_list;
    StationList     st_list;

#include "scan_ap.h"
#include "scan_st.h"
#include "scan_auth.h"
#include "scan_rssi.h"

    // ===== PUBLIC ===== //

    void stop() {
        stopAP();
        stopST();
        stopAuth();
        stopRSSI();
    }

    void printAPs(const result_filter_t* filter) {
        ap_list.print(filter);
    }

    void printSTs(const result_filter_t* filter) {
        st_list.print(filter);
    }

    void print(const result_filter_t* filter) {
        debuglnF("[ ========== Scan Results ========== ]");
        debugln();

        if (!filter || filter->aps) printAPs(filter);
        if (!filter || filter->sts) printSTs(filter);
    }

    void update() {
        update_ap_scan();
        update_st_scan();
        update_auth_scan();
        update_rssi_scan();
    }

    AccessPointList& getAccessPoints() {
        return ap_list;
    }

    StationList& getStations() {
        return st_list;
    }

    bool active(){
        return ap_scan_active() || st_scan_active() || auth_scan_active() || rssi_scan_active();
    }
}