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

SortedStringList& Station::getProbes() {
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
StationList::StationList(int max) : list_max_size(max) {}

StationList::~StationList() {
    clear();
}

bool StationList::push(uint8_t* mac, AccessPoint* ap) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    Station* st = new Station(mac, ap);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = st;
        list_end   = st;
        h          = list_begin;
    } else {
        // Insert at start
        if (memcmp(list_begin->getMAC(), mac, 6) > 0) {
            st->setNext(list_begin);
            list_begin = st;
        }
        // Insert at end
        else if (memcmp(list_end->getMAC(), mac, 6) < 0) {
            list_end->setNext(st);
            list_end = st;
        }
        // Insert somewhere in the middle (insertion sort)
        else {
            Station* tmp_c = list_begin;
            Station* tmp_p = NULL;

            while (tmp_c && memcmp(tmp_c->getMAC(), mac, 6) < 0) {
                tmp_p = tmp_c;
                tmp_c = tmp_c->getNext();
            }

            st->setNext(tmp_c);
            if (tmp_p) tmp_p->setNext(st);
        }
    }
    ++list_size;
    return true;
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
    if (list_begin) {
        Station* h = list_begin;
        int res;

        do {
            res = memcmp(h->getMAC(), mac, 6);
            if (res == 0) return h;
            else h = h->getNext();
        } while (h && res < 0);
    }

    return NULL;
}

Station* StationList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && j<i) {
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