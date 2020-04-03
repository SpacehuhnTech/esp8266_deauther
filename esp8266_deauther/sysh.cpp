/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "sysh.h"

extern "C" {
    #include "user_interface.h"
    typedef void (* freedom_outside_cb_t)(uint8 status);
    int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
    void wifi_unregister_send_pkt_freedom_cb(void);
    int wifi_send_pkt_freedom(uint8* buf, int len, bool sys_seq);
}

#include "config.h"

#ifdef DEBUG_SYSH
#include "debug.h"
#include "strh.h"
#else // ifdef DEBUG_SYSH
#define debug(...) 0
#define debugln(...) 0
#define debugf(...) 0
#define debugF(...) 0
#define debuglnF(...) 0
#endif // ifdef DEBUG_SYSH

namespace sysh {
    void channel(uint8_t ch) {
        if (wifi_get_channel() != ch) {
            wifi_set_channel(ch);
            debugF("[Sysh] Set channel ");
            debugln(String(ch));
        }
    }

    bool send(uint8_t ch, uint8_t* buf, uint16_t len) {
        sysh::channel(ch);
        debuglnF("[Sysh] Send packet");
        return wifi_send_pkt_freedom(buf, len, 0) == 0;
    }

    uint8_t count_ch(uint16_t channels) {
        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        return num_of_channels;
    }

    uint8_t next_ch(uint16_t channels) {
        uint8_t ch = wifi_get_channel();

        // If no channels in register
        // Or the only channel that is, is already set
        if (((channels&0x3FFF) == 0) ||
            (((channels >> (ch-1)) & 0x01) && ((channels & ~(1 << (ch-1))) == 0))) return ch;

        do {
            if (++ch > 14) ch = 1;
        } while (!((channels >> (ch-1)) & 0x01));

        debugF("[Sysh] Get next channel ");
        debugln(String(ch));

        return ch;
    }

    void set_next_ch(uint16_t channels) {
        sysh::channel(next_ch(channels));
    }
}