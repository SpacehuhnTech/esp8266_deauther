/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "StringList.h"
#include "MacArr.h"

typedef struct result_filter_t {
    uint16_t         channels;
    SortedStringList ssids;
    MacArr           bssids;
    SortedStringList vendors;
} result_filter_t;