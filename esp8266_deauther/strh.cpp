/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "strh.h"

namespace strh {
    bool hidden { false };

    void hide_mac(bool mode) {
        hidden = mode;
    }

    String whitespace(int len) {
        String res;

        res.reserve(len);

        while (len > 0) {
            res += ' ';
            --len;
        }

        return res;
    }

    String left(int len, String str) {
        int spaces = len - str.length();

        while (spaces > 0) {
            str = str + ' ';
            --spaces;
        }

        str = str.substring(0, len);

        return str;
    }

    String right(int len, String str) {
        int spaces = len - str.length();

        while (spaces > 0) {
            str = ' ' + str;
            --spaces;
        }

        str = str.substring(0, len);

        return str;
    }

    String center(int len, String str) {
        int spaces = len - str.length();

        for (int i = 0; i < spaces; i += 2) {
            str = ' ' + str + ' ';
        }

        str = str.substring(0, len);

        return str;
    }

    String mac(const uint8_t* b, unsigned int len) {
        len = len - (len % 3);
        String str;

        for (int i = 0; i < len; i++) {
            if (i>0) str += ':';
            if (hidden && (i>=2) && (i<=4)) {
                str += '*';
                str += '*';
            } else {
                if (b[i] < 0x10) str += '0';
                str += String(b[i], HEX);
            }
        }

        return str;
    }

    String escape(String str) {
        if ((str == String('"')) || !str.startsWith("\"") || !str.endsWith("\"")) {
            str.replace("\\", "\\\\");
            str.replace("\"", "\\\"");
            str = '"' + str + '"';
        }
        return str;
    }

    String time(unsigned long time) {
        if (time == 0) return "0ms";

        String str;

        unsigned long second_ms = 1000;
        unsigned long minute_ms = 60*second_ms;
        unsigned long hour_ms   = 60*minute_ms;

        if (time >= hour_ms) {
            unsigned long hours = time/hour_ms;
            time -= hours*hour_ms;
            str  += String(hours)+"h+";
        }

        if (time >= minute_ms) {
            unsigned long minutes = time/minute_ms;
            time -= minutes*minute_ms;
            str  += String(minutes)+"min+";
        }

        if (time >= second_ms) {
            unsigned long seconds = time/second_ms;
            time -= seconds*second_ms;
            str  += String(seconds)+"s+";
        }

        if (time > 0) {
            str += String(time)+"ms+";
        }

        str.remove(str.length()-1);

        return str;
    }

    String channels(uint16_t reg) {
        String str;

        for (uint8_t i = 0; i<14; ++i) {
            if ((reg >> (i)) & 0x01) {
                str += String(i+1);
                str += String(',');
            }
        }
        return str;
    }

    String boolean(bool value) {
        if (value) {
            return String(F("True"));
        } else {
            return String(F("False"));
        }
    }
}