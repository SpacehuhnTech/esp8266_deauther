/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h>

// ========== AccessPoint ========== //

class AccessPoint {
    private:
        char  * ssid;
        uint8_t bssid[6];
        int     rssi;
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
        uint8_t getEncryption() const;
        uint8_t getChannel() const;

        AccessPoint* getNext();

        void setNext(AccessPoint* next);
};

// ========== AccessPointList ========== //

class AccessPointList {
    private:
        AccessPoint* list_begin = NULL;
        AccessPoint* list_end   = NULL;
        int list_size           = 0;

        AccessPoint* h = NULL;

    public:
        ~AccessPointList();

        void push(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch);
        AccessPoint* search(uint8_t* bssid);
        void clear();

        AccessPoint* get(int i);

        void begin();
        AccessPoint* iterate();

        bool available() const;
        int size() const;
};