/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "StringList.h"
#include "packetinjector.h "

namespace attack {
    void beacon(StringList& ssid_list, uint8_t* mac_from, uint8_t* mac_to, Encryption enc, uint8_t ch, unsigned long timeout);
}