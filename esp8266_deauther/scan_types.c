/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "scan_types.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// ===== Access Points ===== //
ap_t* ap_create(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch) {
    ap_t* ap = (ap_t*)malloc(sizeof(ap_t));

    strncpy(ap->ssid, ssid, 32);
    ap->ssid[32] = '\0';

    memcpy(ap->bssid, bssid, 6);

    ap->rssi = rssi;
    ap->enc  = enc;
    ap->ch   = ch;
    ap->next = NULL;

    return ap;
}

void ap_list_push(ap_list_t* list, ap_t* ap) {
    if (list->size < 255) {
        if (!list->begin) {
            list->begin = ap;
            list->end   = ap;
        } else {
            list->end->next = ap;
            list->end       = ap;
        }
        ++(list->size);
    }
}

ap_t* ap_list_search(ap_list_t* list, uint8_t* bssid) {
    ap_t* h = list->begin;

    while (h) {
        if (memcmp(h->bssid, bssid, 6) == 0) {
            return h;
        }
        h = h->next;
    }
    return NULL;
}

void ap_list_clear(ap_list_t* list) {
    ap_t* h = list->begin;

    while (h) {
        ap_t* to_delete = h;
        h = h->next;
        free(to_delete);
    }

    list->begin = NULL;
    list->end   = NULL;
    list->size  = 0;
}

// ===== Stations ===== //
station_t* station_create(uint8_t* mac, ap_t* ap) {
    station_t* s = (station_t*)malloc(sizeof(station_t));

    memcpy(s->mac, mac, 6);
    s->ap   = ap;
    s->pkts = 1;
    s->next = NULL;

    return s;
}

void station_list_push(station_list_t* list, station_t* s) {
    if (list->size < 255) {
        if (!list->begin) {
            list->begin = s;
            list->end   = s;
        } else {
            list->end->next = s;
            list->end       = s;
        }
        ++(list->size);
    }
}

bool station_list_search(station_list_t* list, uint8_t* mac, ap_t* ap) {
    station_t* h = list->begin;

    while (h) {
        if (memcmp(h->mac, mac, 6) == 0) {
            if (ap && !h->ap) h->ap = ap;
            ++(h->pkts);
            return true;
        }
        h = h->next;
    }
    return false;
}

void station_list_clear(station_list_t* list) {
    station_t* h = list->begin;

    while (h) {
        station_t* to_delete = h;
        h = h->next;
        free(to_delete);
    }

    list->begin = NULL;
    list->end   = NULL;
    list->size  = 0;
}