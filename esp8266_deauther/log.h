/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include "config.h"

#include <climits>   // LONG_MAX
#include <Arduino.h> // Serial

#ifdef ENABLE_LOG

#define log_init()\
    LOG_PORT.begin(LOG_BAUD);\
    LOG_PORT.setTimeout(LONG_MAX);\
    LOG_PORT.println();

#define log(...) LOG_PORT.print(__VA_ARGS__)
#define logln(...) LOG_PORT.println(__VA_ARGS__)
#define logf(...) LOG_PORT.printf(__VA_ARGS__)

#else /* ifdef ENABLE_LOG */

#define log_init() 0

#define log(...) 0
#define logln(...) 0
#define logf(...) 0

#endif /* ifdef ENABLE_LOG */