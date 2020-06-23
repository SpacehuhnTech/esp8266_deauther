/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#define VERSION "Version 3.0.0 dev"

// ===== Serial Communication ===== //
#define ENABLE_DEBUG      // Enable serial logging
#define DEBUG_PORT Serial // Serial port
#define DEBUG_BAUD 115200 // Serial baud rate

// ===== Command Line History ===== //
#define ENABLE_HISTORY
#define HISTORY_SIZE 10      // CLI command history

// ===== List Max Sizes ===== //
#define MAX_ACCESS_POINTS 80 // Access point scan results
#define MAX_STATIONS 40      // Station scan results
#define MAX_TARGETS 32       // Deauth targets, Beacons, Probes
#define MAX_PROBES 10        // Probes per station scan result

// ===== EEPROM Settings ===== //
#define EEPROM_SIZE       4095
#define EEPROM_BOOT_ADDR  3210
#define BOOT_MAGIC_NUM    1234567890

// ===== Alias List Settings ===== //
#define ALIAS_ADDR 100
#define ALIAS_MAGIC_NUM 1234567891
#define MAX_ALIAS_NUM 25
#define MAX_ALIAS_LEN 12

// ===== Debug ===== //
// #define DEBUG_SYSH
// #define DEBUG_TARGET_ARR
// #define DEBUG_STRING_LIST
// #define DEBUG_MAC_ARR