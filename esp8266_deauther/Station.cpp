/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "Station.h"
#include "strh.h"
#include "vendor.h"

// ========== Station ========== //

Station::Station(uint8_t* mac, AccessPoint* ap) {
    memcpy(this->mac, mac, 6);
    this->ap = ap;
}

const uint8_t* Station::getMAC() const {
    return mac;
}

String Station::getMACString() const {
    return strh::mac(mac);
}

const AccessPoint* Station::getAccessPoint() const {
    return ap;
}

uint32_t Station::getPackets() const {
    return pkts;
}

String Station::getSSIDString() const {
    if (ap) {
        return ap->getSSIDString();
    } else {
        return String();
    }
}

String Station::getBSSIDString() const {
    if (ap) {
        return ap->getBSSIDString();
    }
    return String();
}

String Station::getVendor() const {
    return vendor::search(mac);
}

StringList& Station::getProbes() {
    return probes;
}

Station* Station::getNext() {
    return next;
}

void Station::setAccessPoint(AccessPoint* ap) {
    this->ap = ap;
}

void Station::newPkt() {
    ++pkts;
}

void Station::setNext(Station* next) {
    this->next = next;
}

// ========== StationList ========== //

StationList::~StationList() {
    clear();
}

void StationList::push(uint8_t* mac, AccessPoint* ap) {
    Station* st = new Station(mac, ap);

    if (!list_begin) {
        list_begin = st;
        list_end   = st;
        h          = list_begin;
    } else {
        list_end->setNext(st);
        list_end = st;
    }
    ++list_size;
}

bool StationList::addProbe(uint8_t* mac, const char* ssid, uint8_t len) {
    // find mac in list
    Station* h = search(mac);

    if (h) {
        // add ssid to list
        String probe;

        for (uint8_t i = 0; i<len; ++i) probe += char(ssid[i]);

        if (!h->getProbes().contains(probe)) {
            h->getProbes().push(probe);
            return true;
        }
    }
    return false;
}

bool StationList::registerPacket(uint8_t* mac, AccessPoint* ap) {
    Station* h = search(mac);

    if (h) {
        if (ap && !h->getAccessPoint()) {
            h->setAccessPoint(ap);
        }
        h->newPkt();
        return false;
    } else {
        push(mac, ap);
        return true;
    }
}

void StationList::clear() {
    Station* h = list_begin;

    while (h) {
        Station* to_delete = h;
        h = h->getNext();
        delete to_delete;
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;
}

Station* StationList::search(uint8_t* mac) {
    Station* h = list_begin;

    while (h && memcmp(h->getMAC(), mac, 6) != 0) {
        h = h->getNext();
    }
    return h;
}

Station* StationList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && i<j) {
        h = h->getNext();
        ++j;
    }

    return h;
}

void StationList::begin() {
    h = list_begin;
}

Station* StationList::iterate() {
    Station* res = h;

    h = h->getNext();

    return res;
}

bool StationList::available() const {
    return h;
}

int StationList::size() const {
    return list_size;
}