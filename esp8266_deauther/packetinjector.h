/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h> // uint8_t, uint16_t

namespace packetinjector {
    bool send(uint8_t ch, uint8_t* buf, uint16_t len);
    bool deauth(uint8_t ch, uint8_t* from, uint8_t* to);
    bool disassoc(uint8_t ch, uint8_t* from, uint8_t* to);
    bool beacon(uint8_t ch, uint8_t* from, uint8_t* to, const char* ssid, bool wpa2);
    bool probe(uint8_t ch, uint8_t* from, const char* ssid);
}