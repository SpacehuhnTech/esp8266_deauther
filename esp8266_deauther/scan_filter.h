/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "StringList.h"
#include "MACList.h"

typedef struct scan_filter_t {
    uint16_t          channels;
    const StringList* ssids;
    const MACList   * bssid;
    const StringList* vendors;
} scan_filter_t;