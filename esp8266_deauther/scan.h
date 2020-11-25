/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "AccessPoint.h"
#include "Station.h"
#include "result_filter.h"

// typedef void (* rssi_cb_f)(int8_t);

typedef struct st_scan_settings_t {
    uint16_t      channels;
    unsigned long ch_time;
    unsigned long timeout;
    bool          retain;
} st_scan_settings_t;

typedef struct ap_scan_settings_t {
    uint16_t           channels;
    bool               retain;
    bool               st;
    st_scan_settings_t st_settings;
} ap_scan_settings_t;

typedef struct auth_scan_settings_t {
    uint16_t      channels;
    unsigned long ch_time;
    unsigned long timeout;
    bool          beacon;
    MacArr        bssids;
    bool          save;
} auth_scan_settings_t;

typedef struct rssi_scan_settings_t {
    uint16_t      channels;
    unsigned long ch_time;
    unsigned long update_time;
    MacArr        macs;
} rssi_scan_settings_t;

const st_scan_settings_t   ST_SCAN_DEFAULT { 0x3FFF, 284, 20000, false };
const ap_scan_settings_t   AP_SCAN_DEFAULT { 0x3FFF, false, false, ST_SCAN_DEFAULT };
const auth_scan_settings_t AUTH_SCAN_DEFAULT { 0x3FFF, 284, 20000, false, MacArr {}, false };
const rssi_scan_settings_t RSSI_SCAN_DEFAULT { 0x3FFF, 284, 5000, MacArr {} };

namespace scan {
    void clearAPresults();
    void clearSTresults();

    void startAP(const ap_scan_settings_t& settings     = AP_SCAN_DEFAULT);
    void startST(const st_scan_settings_t& settings     = ST_SCAN_DEFAULT);
    void startAuth(const auth_scan_settings_t& settings = AUTH_SCAN_DEFAULT);
    void startRSSI(const rssi_scan_settings_t& settings = RSSI_SCAN_DEFAULT);

    void stopAP();
    void stopST();
    void stopAuth();
    void stopRSSI();
    void stop();

    void print(const result_filter_t* filter    = nullptr);
    void printAPs(const result_filter_t* filter = nullptr);
    void printSTs(const result_filter_t* filter = nullptr);

    void update();

    AccessPointList& getAccessPoints();
    StationList& getStations();

    bool active();
}