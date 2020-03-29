/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "AccessPoint.h"
#include "Station.h"
#include "scan_filter.h"

typedef void (* rssi_cb_f)(int8_t);

namespace scan {
    void clearAPresults();
    void clearSTresults();

    void start(bool ap = true, bool st = true, unsigned long timeout = 14000, uint16_t channels = 0x3FFF, unsigned long ch_time = 1000, bool silent = false, bool retain = false);

    void startRSSI(rssi_cb_f rssi_cb, MACList& mac_filter, uint16_t channels = 0x3FFF, unsigned long ch_time = 1000);
    void startAuth(uint8_t* mac, unsigned long timeout, bool silent);

    void stop();

    void printAPs(const result_filter_t* filter = nullptr);
    void printSTs(const result_filter_t* filter = nullptr);

    void update();

    AccessPointList& getAccessPoints();
    StationList    & getStations();
}