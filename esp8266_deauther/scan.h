/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "AccessPoint.h"
#include "Station.h"

namespace scan {
    void clearAPresults();
    void clearSTresults();

    void search(bool ap, bool st, unsigned long time, uint16_t channels, bool retain);
    void searchAPs();
    void searchSTs(unsigned long time, uint16_t channels);

    void printAPs();
    void printSTs();
    void printResults();

    AccessPointList& getAccessPoints();
    StationList    & getStations();
}