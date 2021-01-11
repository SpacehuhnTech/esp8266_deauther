/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

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
#define debugF(...) DEBUG_PORT.print(F(__VA_ARGS__))
#define debuglnF(...) DEBUG_PORT.println(F(__VA_ARGS__))

#define debug_available() DEBUG_PORT.available()
#define debug_read() DEBUG_PORT.read()
#define debug_peek() DEBUG_PORT.peek()

#else /* ifdef ENABLE_DEBUG */

#define debug_init() 0

#define debug(...) 0
#define debugln(...) 0
#define debugf(...) 0
#define debugF(...) 0
#define debuglnF(...) 0

#define debug_available() 0
#define debug_read() 0
#define debug_peek() 0

#endif /* ifdef ENABLE_DEBUG */