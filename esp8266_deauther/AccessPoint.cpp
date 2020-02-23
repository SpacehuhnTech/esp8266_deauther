/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "AccessPoint.h"
#include "strh.h"
#include "vendor.h"

#include <ESP8266WiFi.h>

// ========== AccessPoint ========== //

AccessPoint::AccessPoint(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch) {
    if (ssid) {
        int ssidlen = strlen(ssid);

        if (ssidlen > 0) {
            if (ssidlen > 32) ssidlen = 32;

            this->ssid = (char*)malloc(ssidlen+1);
            memcpy(this->ssid, ssid, ssidlen+1);
            this->ssid[ssidlen] = '\0';
        }
    }

    memcpy(this->bssid, bssid, 6);
    this->rssi = rssi;
    this->enc  = enc;
    this->ch   = ch;
}

AccessPoint::~AccessPoint() {
    if (ssid) {
        free(ssid);
        this->ssid = NULL;
    }
}

const char* AccessPoint::getSSID() const {
    return ssid;
}

const uint8_t* AccessPoint::getBSSID() const {
    return bssid;
}

String AccessPoint::getSSIDString() const {
    if (hidden()) {
        return "*HIDDEN-NETWORK*";
    } else {
        return '"' + String(ssid) + '"';
    }
}

String AccessPoint::getBSSIDString() const {
    return strh::mac(bssid);
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

bool AccessPoint::hidden() const {
    return !ssid;
}

String AccessPoint::getVendor() const {
    return vendor::search(bssid);
}

AccessPoint* AccessPoint::getNext() {
    return next;
}

void AccessPoint::setNext(AccessPoint* next) {
    this->next = next;
}

// ========== AccessPointList ========== //

AccessPointList::~AccessPointList() {
    clear();
}

void AccessPointList::push(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch) {
    AccessPoint* ap = new AccessPoint(ssid, bssid, rssi, enc, ch);

    if (!list_begin) {
        list_begin = ap;
        list_end   = ap;
        h          = list_begin;
    } else {
        list_end->setNext(ap);
        list_end = ap;
    }
    ++list_size;
}

AccessPoint* AccessPointList::search(uint8_t* bssid) {
    AccessPoint* h = list_begin;

    while (h) {
        if (memcmp(h->getBSSID(), bssid, 6) == 0) {
            return h;
        }
        h = h->getNext();
    }
    return NULL;
}

void AccessPointList::clear() {
    AccessPoint* h = list_begin;

    while (h) {
        AccessPoint* to_delete = h;
        h = h->getNext();
        delete to_delete;
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;
}

AccessPoint* AccessPointList::get(int i) {
    h = list_begin;
    int j = 0;

    while (h && i<j) {
        h = h->getNext();
        ++j;
    }

    return h;
}

void AccessPointList::begin() {
    h = list_begin;
}

AccessPoint* AccessPointList::iterate() {
    AccessPoint* res = h;

    h = h->getNext();

    return res;
}

bool AccessPointList::available() const {
    return h;
}

int AccessPointList::size() const {
    return list_size;
}