/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "config.h"

#include <climits>   // LONG_MAX
#include <Arduino.h> // Serial

#ifdef ENABLE_DEBUG

#define debug_init()\
    DEBUG_PORT.begin(DEBUG_BAUD);\
    DEBUG_PORT.setTimeout(LONG_MAX);\
    DEBUG_PORT.println();

#define debug(...) DEBUG_PORT.print(__VA_ARGS__)
#define debugln(...) DEBUG_PORT.println(__VA_ARGS__)
#define debugf(...) DEBUG_PORT.printf(__VA_ARGS__)

#define debug_update()\
    if (DEBUG_PORT.available()) {\
        String input = DEBUG_PORT.readStringUntil('\n');\
        cli::parse(input.c_str());\
    }

static inline bool debug_busy_wait() {
    if (DEBUG_PORT.available()) {
        String input = DEBUG_PORT.readStringUntil('\n');
        return input == "stop" || input == "exit";
    }
    return false;
}

#else /* ifdef ENABLE_DEBUG */

#define debug_init() 0

#define debug(...) 0
#define debugln(...) 0
#define debugf(...) 0

#define debug_update() 0

static inline bool debug_busy_wait() {
    return false;
}

#endif /* ifdef ENABLE_DEBUG */