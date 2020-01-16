/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>

extern "C" {
  #include "scan_types.h"
}

namespace scan {
    void clearAPresults();
    void clearSTresults();

    void searchAPs();
    void searchSTs(unsigned long time, uint16_t channels);

    void printAPs();
    void printSTs();
    void printResults();

    ap_t* getAP(int id);
    station_t* getStation(int id);
}