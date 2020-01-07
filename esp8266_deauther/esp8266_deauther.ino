/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "log.h"

void setup() {
    log_init();
}

void loop() {
    logln("Hello World!");
    delay(1000);
}