/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// ===== Serial Communication ===== //
#define ENABLE_DEBUG      // Enable serial logging
#define DEBUG_PORT Serial // Serial port
#define DEBUG_BAUD 115200 // Serial baud rate

// ===== List Max Sizes ===== //
#define MAX_STATIONS 40
#define MAX_ACCESS_POINTS 80
#define MAX_TARGETS 50
#define MAX_PROBES 10