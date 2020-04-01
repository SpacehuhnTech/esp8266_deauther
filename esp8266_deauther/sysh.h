/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stddef.h> // integers

extern "C" {
    #include "user_interface.h"
    typedef void (* freedom_outside_cb_t)(uint8 status);
    int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
    void wifi_unregister_send_pkt_freedom_cb(void);
    int wifi_send_pkt_freedom(uint8* buf, int len, bool sys_seq);
}

namespace sysh {
    inline void channel(uint8_t ch) {
        wifi_set_channel(ch);
    }

    inline bool send(uint8_t ch, uint8_t* buf, uint16_t len) {
        sysh::channel(ch);
        return wifi_send_pkt_freedom(buf, len, 0) == 0;
    }

    uint8_t count_ch(uint16_t channels);
    uint8_t next_ch(uint16_t channels);

    inline void set_next_ch(uint16_t channels) {
        sysh::channel(next_ch(channels));
    }
}