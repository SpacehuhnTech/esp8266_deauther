/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "AccessPoint.h"
#include "StringList.h"
#include "config.h"

// ========== Station ========== //

class Station {
    private:
        uint8_t mac[6];
        AccessPoint* ap;

        uint32_t pkts           = 1;
        SortedStringList probes = SortedStringList(MAX_PROBES);
        Station* next           = NULL;

    public:
        Station(uint8_t* mac, AccessPoint* ap);

        const uint8_t* getMAC() const;
        String getMACString() const;
        const AccessPoint* getAccessPoint() const;
        uint32_t getPackets() const;
        String getSSIDString() const;
        String getBSSIDString() const;
        String getVendor() const;

        SortedStringList& getProbes();
        Station* getNext();

        void setAccessPoint(AccessPoint* ap);
        void newPkt();
        void setNext(Station* next);

        bool operator==(const uint8_t* mac) const;
        bool operator<(const uint8_t* mac) const;
        bool operator>(const uint8_t* mac) const;
};

// ========== StationList ========== //

class StationList {
    private:
        Station* list_begin = NULL;
        Station* list_end   = NULL;
        int list_size       = 0;
        int list_max_size;

        Station* list_h = NULL;
        int list_pos    = 0;

    public:
        StationList(int max = MAX_STATIONS);
        ~StationList();

        bool push(uint8_t* mac, AccessPoint* ap);
        bool addProbe(uint8_t* mac, const char* ssid, uint8_t len);
        bool registerPacket(uint8_t* mac, AccessPoint* ap);
        void clear();

        Station* search(uint8_t* mac);
        Station* get(int i);

        void begin();
        Station* iterate();

        bool available() const;
        int size() const;
        bool full() const;
};