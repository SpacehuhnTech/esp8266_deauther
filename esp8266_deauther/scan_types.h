/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#ifndef scan_types_h
#define scan_types_h

#include <stdint.h>
#include <stdbool.h>

// ===== Access Points ===== //
typedef struct ap_t      ap_t;
typedef struct ap_list_t ap_list_t;

struct ap_t {
    char    ssid[33];
    uint8_t bssid[6];
    int     rssi;
    uint8_t enc;
    uint8_t ch;
    ap_t  * next;
};

struct ap_list_t {
    ap_t* begin;
    ap_t* end;
    int   size;
};

ap_t* ap_create(const char* ssid, uint8_t* bssid, int rssi, uint8_t enc, uint8_t ch);
void ap_list_push(ap_list_t* list, ap_t* ap);
ap_t* ap_list_search(ap_list_t* list, uint8_t* bssid);
void ap_list_clear(ap_list_t* list);

// ===== Stations ===== //
typedef struct station_t      station_t;
typedef struct station_list_t station_list_t;

struct station_t {
    uint8_t    mac[6];
    ap_t     * ap;
    uint32_t   pkts;
    station_t* next;
};

struct station_list_t {
    station_t* begin;
    station_t* end;
    int        size;
};

station_t* station_create(uint8_t* mac, ap_t* ap);
void station_list_push(station_list_t* list, station_t* s);
bool station_list_search(station_list_t* list, uint8_t* mac, ap_t* ap);
void station_list_clear(station_list_t* list);

#endif /* ifndef scan_types_h */