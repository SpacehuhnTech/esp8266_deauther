/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

namespace scan {
    void begin();

    void searchAPs();
    void searchSTs(unsigned long time);

    void printAPs();
    void printStations();
    void printResults();
}