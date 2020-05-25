/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "config.h"
#include "AccessPoint.h"
#include "Station.h"

typedef struct target_t {
    uint8_t  sender[6];
    uint8_t  receiver[6];
    uint16_t channels;
} target_t;

class TargetArr {
    private:
        typedef struct target_list_t {
            target_t   * data;
            unsigned int max;
            unsigned int size;
            unsigned int h;
        } target_list_t;

        target_list_t list { nullptr, 0, 0, 0 };

        bool contains(const target_t& t);

    public:
        TargetArr(int size = 0);
        TargetArr(const TargetArr& ml);
        TargetArr(TargetArr&& ml);
        ~TargetArr();

        TargetArr& operator=(const TargetArr& tl);
        TargetArr& operator=(TargetArr&& tl);

        TargetArr& operator+=(const TargetArr& tl);

        void clear();

        bool add(const uint8_t* sender, const uint8_t* receiver, uint16_t channels);
        bool add(const AccessPoint* ap);
        bool add(const Station* st);

        void begin();
        const target_t* iterate();

        bool available() const;
        int size() const;
        bool full() const;
};