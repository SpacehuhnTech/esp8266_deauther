/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
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

int StationList::compare(const Station* st, const uint8_t* mac) const {
    return memcmp(st->getMAC(), mac, 6);
}

StationList::StationList(int max) : list_max_size(max) {}

StationList::~StationList() {
    clear();
}

bool StationList::push(uint8_t* mac, AccessPoint* ap) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    Station* new_st = new Station(mac, ap);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_st;
        list_end   = new_st;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (compare(list_begin, mac) > 0) {
            new_st->setNext(list_begin);
            list_begin = new_st;
        }
        // Insert at end
        else if (compare(list_end, mac) < 0) {
            list_end->setNext(new_st);
            list_end = new_st;
        }
        // Insert somewhere in the middle (insertion sort)
        else {
            Station* tmp_c = list_begin;
            Station* tmp_p = NULL;

            int res;

            do {
                res   = compare(tmp_c, mac);
                tmp_p = tmp_c;
                tmp_c = tmp_c->getNext();
            } while (tmp_c && res < 0);

            // Skip duplicates
            if (res == 0) {
                delete new_st;
                return false;
            } else {
                new_st->setNext(tmp_c);
                if (tmp_p) tmp_p->setNext(new_st);
            }
        }
    }

    ++list_size;
    return true;
}

bool StationList::addProbe(uint8_t* mac, const char* ssid, uint8_t len) {
    // find mac in list
    Station* tmp = search(mac);

    if (tmp) {
        // add ssid to list
        String probe;

        for (uint8_t i = 0; i<len; ++i) probe += char(ssid[i]);

        if (!tmp->getProbes().contains(probe)) {
            tmp->getProbes().push(probe);
            return true;
        }
    }
    return false;
}

bool StationList::registerPacket(uint8_t* mac, AccessPoint* ap) {
    Station* tmp = search(mac);

    if (tmp) {
        if (ap && !tmp->getAccessPoint()) {
            tmp->setAccessPoint(ap);
        }
        tmp->newPkt();
        return false;
    } else {
        push(mac, ap);
        return true;
    }
}

void StationList::clear() {
    Station* tmp = list_begin;

    while (tmp) {
        Station* to_delete = tmp;
        tmp = tmp->getNext();
        delete to_delete;
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    list_h   = NULL;
    list_pos = 0;
}

Station* StationList::search(uint8_t* mac) {
    if ((list_size == 0) || (compare(list_begin, mac) > 0) || (compare(list_end, mac) < 0)) {
        return NULL;
    }

    Station* tmp = list_begin;

    int res = compare(tmp, mac);

    while (tmp && res < 0) {
        tmp = tmp->getNext();
    }

    return (res == 0) ? tmp : NULL;
}

Station* StationList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return list_h;
}

void StationList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

Station* StationList::iterate() {
    Station* tmp = list_h;

    if (list_h) {
        list_h = list_h->getNext();
        ++list_pos;
    }

    return tmp;
}

bool StationList::available() const {
    return list_h;
}

int StationList::size() const {
    return list_size;
}

bool StationList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}