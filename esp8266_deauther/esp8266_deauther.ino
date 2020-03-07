/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "debug.h"
#include "cli.h"
#include "scan.h"
#include "attack.h"
#include "eeprom.h"

void setup() {
    cli::begin();
    eeprom::begin();
}

void loop() {
    cli::update();
    attack::update();
    scan::update();
}