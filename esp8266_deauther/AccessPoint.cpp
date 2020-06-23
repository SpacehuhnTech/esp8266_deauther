/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "AccessPoint.h"
#include "strh.h"
#include "vendor.h"
#include "alias.h"
#include "debug.h"

#include <ESP8266WiFi.h>

// ========== AccessPoint ========== //

AccessPoint::AccessPoint(bool hidden, const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch) {
    this->hidden = hidden;

    if (ssid) {
        int ssidlen = strlen(ssid);

        if (ssidlen > 0) {
            if (ssidlen > 32) ssidlen = 32;

            this->ssid = new char[ssidlen+1];
            memcpy(this->ssid, ssid, ssidlen);
            this->ssid[ssidlen] = '\0';
        }
    }

    if (bssid) memcpy(this->bssid, bssid, 6);
    this->rssi = rssi;
    this->enc  = enc;
    this->ch   = ch;
}

AccessPoint::~AccessPoint() {
    if (ssid) {
        delete[] ssid;
        ssid = nullptr;
    }
}

const char* AccessPoint::getSSID() const {
    return ssid;
}

const uint8_t* AccessPoint::getBSSID() const {
    return bssid;
}

String AccessPoint::getSSIDString() const {
    if (isHidden()) {
        return "*HIDDEN-NETWORK*";
    } else {
        return '"' + String(ssid) + '"';
    }
}

String AccessPoint::getBSSIDString() const {
    return alias::get(bssid);
}

int AccessPoint::getRSSI() const {
    return rssi;
}

String AccessPoint::getEncryption() const {
    switch (enc) {
        case ENC_TYPE_NONE:
            return "Open";
        case ENC_TYPE_WEP:
            return "WEP";
        case ENC_TYPE_TKIP:
            return "WPA";
        case ENC_TYPE_CCMP:
            return "WPA2";
        case ENC_TYPE_AUTO:
            return "WPA*";
        default:
            return "?";
    }
}

uint8_t AccessPoint::getChannel() const {
    return ch;
}

bool AccessPoint::isHidden() const {
    return hidden;
}

String AccessPoint::getVendor() const {
    return vendor::getName(bssid);
}

AccessPoint* AccessPoint::getNext() {
    return next;
}

void AccessPoint::setNext(AccessPoint* next) {
    this->next = next;
}

void AccessPoint::print(int id, const result_filter_t* f) {
    if (f) {
        if (((f->channels >> (getChannel()-1)) & 1) == 0) return;
        if (!f->ssids.empty() && !f->ssids.contains(getSSID(),false)) return;
        if (!f->bssids.empty() && !f->bssids.contains(getBSSID())) return;
        if (!f->vendors.empty() && !f->vendors.contains(getVendor(),false)) return;
    }

    debug(strh::right(3, String(id)));
    debug(' ');
    debug(strh::left(34, getSSIDString()));
    debug(' ');
    debug(strh::right(4, String(getRSSI())));
    debug(' ');
    debug(strh::left(4, getEncryption()));
    debug(' ');
    debug(strh::right(2, String(getChannel())));
    debug(' ');
    debug(strh::left(17, getBSSIDString()));
    debug(' ');
    debug(strh::left(8, getVendor()));
    debugln();
}

// ========== AccessPointList ========== //

int AccessPointList::compare(const AccessPoint* ap, const uint8_t* bssid) const {
    return memcmp(ap->getBSSID(), bssid, 6);
}

AccessPointList::AccessPointList(int max) : list_max_size(max) {}

AccessPointList::~AccessPointList() {
    clear();
}

bool AccessPointList::push(bool hidden, const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    AccessPoint* new_ap = new AccessPoint(hidden, ssid, bssid, rssi, enc, ch);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_ap;
        list_end   = new_ap;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (compare(list_begin, bssid) > 0) {
            new_ap->setNext(list_begin);
            list_begin = new_ap;
        }
        // Insert at end
        else if (compare(list_end, bssid) < 0) {
            list_end->setNext(new_ap);
            list_end = new_ap;
        }
        // Insert somewhere in the middle (insertion sort)
        else {
            AccessPoint* tmp_c = list_begin;
            AccessPoint* tmp_p = nullptr;

            int res = compare(tmp_c, bssid);

            while (tmp_c->getNext() && res < 0) {
                tmp_p = tmp_c;
                tmp_c = tmp_c->getNext();
                res   = compare(tmp_c, bssid);
            }

            // Skip duplicates
            if (res == 0) {
                delete new_ap;
                return false;
            } else {
                new_ap->setNext(tmp_c);
                if (tmp_p) tmp_p->setNext(new_ap);
            }
        }
    }
    ++list_size;
    return true;
}

AccessPoint* AccessPointList::search(const uint8_t* bssid) {
    if ((list_size == 0) || (compare(list_begin, bssid) > 0) || (compare(list_end, bssid) < 0)) {
        return nullptr;
    }

    AccessPoint* tmp = list_begin;
    int res          = compare(tmp, bssid);

    while (tmp->getNext() && res < 0) {
        tmp = tmp->getNext();
        res = compare(tmp, bssid);
    }

    return (res == 0) ? tmp : nullptr;
}

void AccessPointList::clear() {
    AccessPoint* tmp = list_begin;

    while (tmp) {
        AccessPoint* to_delete = tmp;
        tmp = tmp->getNext();
        delete to_delete;
    }

    list_begin = nullptr;
    list_end   = nullptr;
    list_size  = 0;

    list_h   = nullptr;
    list_pos = 0;
}

AccessPoint* AccessPointList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return list_h;
}

void AccessPointList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

AccessPoint* AccessPointList::iterate() {
    AccessPoint* tmp = list_h;

    if (list_h) {
        list_h = list_h->getNext();
        ++list_pos;
    }

    return tmp;
}

bool AccessPointList::available() const {
    return list_h;
}

int AccessPointList::size() const {
    return list_size;
}

bool AccessPointList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}

void AccessPointList::printHeader() {
    debuglnF("[ ===== Access Points ===== ]");
    debuglnF(" ID SSID (Network Name)                RSSI Mode Ch BSSID (MAC Addr.) Vendor");
    debuglnF("==============================================================================");
}

void AccessPointList::printFooter() {
    debuglnF("==============================================================================");
    debuglnF("Ch = 2.4 GHz Channel  ,  RSSI = Signal strengh  ,  WPA* = WPA & WPA2 auto mode");
    debuglnF("WPA(2) Enterprise networks are recognized as Open");
    debuglnF("==============================================================================");

    debugln();
}

void AccessPointList::print(const result_filter_t* filter) {
    if (size() == 0) {
        debuglnF("No access points found. Type 'scan ap' to search.");
        debugln();
    } else {
        printHeader();

        int i = 0;
        begin();

        while (available()) {
            iterate()->print(i, filter);
            ++i;
        }

        printFooter();
    }
}