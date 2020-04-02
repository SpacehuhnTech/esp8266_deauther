/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

namespace sysh {
    void channel(uint8_t ch);
    bool send(uint8_t ch, uint8_t* buf, uint16_t len);

    uint8_t count_ch(uint16_t channels);
    uint8_t next_ch(uint16_t channels);

    void set_next_ch(uint16_t channels);
}