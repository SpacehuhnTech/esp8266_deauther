/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h>
#include "config.h"

// ========== AccessPoint ========== //

class AccessPoint {
    private:
        char* ssid;
        uint8_t bssid[6];
        int rssi;
        uint8_t enc;
        uint8_t ch;
        AccessPoint* next = NULL;

    public:
        AccessPoint(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch);
        ~AccessPoint();

        const char* getSSID() const;
        const uint8_t* getBSSID() const;
        String getSSIDString() const;
        String getBSSIDString() const;
        int getRSSI() const;
        String getEncryption() const;
        uint8_t getChannel() const;
        bool hidden() const;
        String getVendor() const;

        AccessPoint* getNext();

        void setNext(AccessPoint* next);
};

// ========== AccessPointList ========== //

class AccessPointList {
    private:
        AccessPoint* list_begin = NULL;
        AccessPoint* list_end   = NULL;
        int list_size           = 0;
        int list_max_size;

        AccessPoint* h = NULL;

    public:
        AccessPointList(int max = MAX_ACCESS_POINTS);
        ~AccessPointList();

        bool push(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch);
        AccessPoint* search(uint8_t* bssid);
        void clear();

        AccessPoint* get(int i);

        void begin();
        AccessPoint* iterate();

        bool available() const;
        int size() const;
        bool full() const;
};