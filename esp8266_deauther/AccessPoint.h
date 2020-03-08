/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <Arduino.h>
#include "config.h"
#include "StringList.h"

#ifndef MAX_ACCESS_POINTS
#define MAX_ACCESS_POINTS 0
#endif // ifndef MAX_ALIAS

// ========== AccessPoint ========== //

class AccessPoint {
    private:
        bool hidden;
        char* ssid;
        uint8_t bssid[6];
        int rssi;
        uint8_t enc;
        uint8_t ch;
        AccessPoint* next = NULL;

    public:
        AccessPoint(bool hidden, const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch);
        ~AccessPoint();

        const char* getSSID() const;
        const uint8_t* getBSSID() const;
        String getSSIDString() const;
        String getBSSIDString() const;
        int getRSSI() const;
        String getEncryption() const;
        uint8_t getChannel() const;
        bool isHidden() const;
        String getVendor() const;

        AccessPoint* getNext();

        void setNext(AccessPoint* next);

        void print(unsigned int id, uint16_t channels = 0x3FFF, const StringList* ssids = NULL, const uint8_t* bssid = NULL);
};

// ========== AccessPointList ========== //

class AccessPointList {
    private:
        AccessPoint* list_begin = NULL;
        AccessPoint* list_end   = NULL;

        int list_size = 0;
        int list_max_size;

        AccessPoint* list_h = NULL;
        int list_pos        = 0;

        int compare(const AccessPoint* ap, const uint8_t* bssid) const;

    public:
        AccessPointList(int max = MAX_ACCESS_POINTS);
        ~AccessPointList();

        bool push(bool hidden, const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch);
        AccessPoint* search(const uint8_t* bssid);
        void clear();

        AccessPoint* get(int i);

        void begin();
        AccessPoint* iterate();

        bool available() const;
        int size() const;
        bool full() const;

        void print(uint16_t channels = 0x3FFF, const StringList* ssids = NULL, const uint8_t* bssid = NULL);
};