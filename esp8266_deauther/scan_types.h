/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

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

// ===== Probes ===== //
typedef struct probe_t      probe_t;
typedef struct probe_list_t probe_list_t;

struct probe_t {
    char   * ssid;
    probe_t* next;
};

struct probe_list_t {
    probe_t* begin;
    probe_t* end;
    int      size;
};

probe_t* probe_create(const char* str, uint8_t len);
void probe_destroy(probe_t* probe);

probe_list_t* probe_list_create();
void probe_list_push(probe_list_t* list, probe_t* probe);
bool probe_list_contains(probe_list_t* list, const char* ssid, uint8_t len);
void probe_list_clear(probe_list_t* list);
void probe_list_destroy(probe_list_t* list);

// ===== Stations ===== //
typedef struct station_t      station_t;
typedef struct station_list_t station_list_t;

struct station_t {
    uint8_t       mac[6];
    ap_t        * ap;
    uint32_t      pkts;
    probe_list_t* probes;
    station_t   * next;
};

struct station_list_t {
    station_t* begin;
    station_t* end;
    int        size;
};

station_t* station_create(uint8_t* mac, ap_t* ap);
void station_list_push(station_list_t* list, station_t* s);
void station_list_push_probe(station_list_t* list, uint8_t* mac, const char* ssid, uint8_t len);
bool station_list_contains(station_list_t* list, uint8_t* mac, ap_t* ap);
void station_list_clear(station_list_t* list);