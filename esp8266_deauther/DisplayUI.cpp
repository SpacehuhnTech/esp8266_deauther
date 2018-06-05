#include "DisplayUI.h"

DisplayUI::DisplayUI() {}

void DisplayUI::setupDisplay() {
    // ===== adjustable ===== //
    // initialize display
    display.init();

    /*
       In case of a compiler (conversion char/uint8_t) error,
       make sure to have version 4 of the display library installed
       https://github.com/ThingPulse/esp8266-oled-ssd1306/releases/tag/4.0.0
     */
    display.setFont(DejaVu_Sans_Mono_12);
    display.setContrast(255);
#ifndef FLIP_DIPLAY
    display.flipScreenVertically();
#endif // ifndef FLIP_DIPLAY
    display.clear();
    display.display();
    // ====================== //
}

void DisplayUI::on() {
    // ===== adjustable ===== //
    if (enabled) {
        display.displayOn();
        mode          = SCREEN_MODE_MENU;
        buttonUp.time = currentTime; // update a button time to keep display on
        prntln(D_MSG_DISPLAY_ON);
    } else {
        prntln(D_ERROR_NOT_ENABLED);
    }
    // ====================== //
}

void DisplayUI::off() {
    // ===== adjustable ===== //
    if (enabled) {
        display.displayOff();
        mode = SCREEN_MODE_OFF;
        prntln(D_MSG_DISPLAY_OFF);
    } else {
        prntln(D_ERROR_NOT_ENABLED);
    }
    // ====================== //
}

void DisplayUI::setupButtons() {
    // ===== adjustable ===== //

#ifdef BUTTON_UP
    buttonUp.enabled = true;
    buttonUp.gpio    = BUTTON_UP;
#else // ifdef BUTTON_UP
    buttonUp.enabled = false;
#endif // ifdef BUTTON_UP

#ifdef BUTTON_DOWN
    buttonDown.enabled = true;
    buttonDown.gpio    = BUTTON_DOWN;
#else // ifdef BUTTON_DOWN
    buttonDown.enabled = false;
#endif // ifdef BUTTON_DOWN

#ifdef BUTTON_LEFT
    buttonLeft.enabled = true;
    buttonLeft.gpio    = BUTTON_LEFT;
#else // ifdef BUTTON_LEFT
    buttonLeft.enabled = false;
#endif // ifdef BUTTON_LEFT

#ifdef BUTTON_RIGHT
    buttonRight.enabled = true;
    buttonRight.gpio    = BUTTON_RIGHT;
#else // ifdef BUTTON_RIGHT
    buttonRight.enabled = false;
#endif // ifdef BUTTON_RIGHT

#ifdef BUTTON_A
    buttonA.enabled = true;
    buttonA.gpio    = BUTTON_A;
#else // ifdef BUTTON_A
    buttonA.enabled = false;
#endif // ifdef BUTTON_A

#ifdef BUTTON_B
    buttonB.enabled = true;
    buttonB.gpio    = BUTTON_B;
#else // ifdef BUTTON_B
    buttonB.enabled = false;
#endif // ifdef BUTTON_B

    // ====================== //
    // setup and read functions
    // ====================== //
    buttonUp.setup = [this]() {
                         if (buttonUp.gpio != 2) pinMode(buttonUp.gpio, INPUT_PULLUP);
                         buttonUp.time = currentTime;
                     };
    buttonUp.read = [this]() {
                        return !digitalRead(buttonUp.gpio);
                    };

    buttonDown.setup = [this]() {
                           if (buttonDown.gpio != 2) pinMode(buttonDown.gpio, INPUT_PULLUP);
                           buttonDown.time = currentTime;
                       };
    buttonDown.read = [this]() {
                          return !digitalRead(buttonDown.gpio);
                      };

    buttonLeft.setup = [this]() {
                           if (buttonLeft.gpio != 2) pinMode(buttonLeft.gpio, INPUT_PULLUP);
                           buttonLeft.time = currentTime;
                       };
    buttonLeft.read = [this]() {
                          return !digitalRead(buttonLeft.gpio);
                      };

    buttonRight.setup = [this]() {
                            if (buttonRight.gpio != 2) pinMode(buttonRight.gpio, INPUT_PULLUP);
                            buttonRight.time = currentTime;
                        };
    buttonRight.read = [this]() {
                           return !digitalRead(buttonRight.gpio);
                       };

    buttonA.setup = [this]() {
                        if (buttonA.gpio != 2) pinMode(buttonA.gpio, INPUT_PULLUP);
                        buttonA.time = currentTime;
                    };
    buttonA.read = [this]() {
                       return !digitalRead(buttonA.gpio);
                   };

    buttonB.setup = [this]() {
                        if (buttonB.gpio != 2) pinMode(buttonB.gpio, INPUT_PULLUP);
                        buttonB.time = currentTime;
                    };
    buttonB.read = [this]() {
                       return !digitalRead(buttonB.gpio);
                   };

    // ====================== //
}

void DisplayUI::setup() {
    setupDisplay();
    setupButtons();

    // ===== PUSH AND RELEASE FUNCTIONS ===== //

    // === BUTTON UP === //
    buttonUp.push = [this]() {
                        if (buttonUp.time > currentTime - BUTTON_DELAY) return;

                        buttonUp.pushed = true;
                        buttonUp.time   = currentTime;
                        scrollCounter   = 0;

                        if (mode == SCREEN_MODE_MENU) {                 // when in menu, go up or down with cursor
                            if (currentMenu->selected > 0) currentMenu->selected--;
                            else currentMenu->selected = currentMenu->list->size() - 1;
                        } else if (mode == SCREEN_MODE_PACKETMONITOR) { // when in packet monitor, change channel
                            scan.setChannel(wifi_channel + 1);
                        }
                    };

    buttonUp.release = [this]() {
                           if (!buttonUp.pushed) return;

                           buttonUp.pushed = false;
                       };

    // === BUTTON DOWN === //
    buttonDown.push = [this]() {
                          if (buttonDown.time > currentTime - BUTTON_DELAY) return;

                          buttonDown.pushed = true;
                          buttonDown.time   = currentTime;
                          scrollCounter     = 0;

                          if (mode == SCREEN_MODE_MENU) {                 // when in menu, go up or down with cursor
                              if (currentMenu->selected < currentMenu->list->size() - 1) currentMenu->selected++;
                              else currentMenu->selected = 0;
                          } else if (mode == SCREEN_MODE_PACKETMONITOR) { // when in packet monitor, change channel
                              scan.setChannel(wifi_channel - 1);
                          }
                      };

    buttonDown.release = [this]() {
                             if (!buttonDown.pushed) return;

                             buttonDown.pushed = false;
                         };

    // === BUTTON LEFT === //
    buttonLeft.push = [this]() {
                          if (buttonLeft.time > currentTime - BUTTON_DELAY) return;

                          buttonLeft.pushed = true;
                          buttonLeft.time   = currentTime;
                          scrollCounter     = 0;
                      };

    buttonLeft.release = [this]() {
                             if (!buttonLeft.pushed) return;

                             buttonLeft.pushed = false;
                         };

    // === BUTTON RIGHT === //
    buttonRight.push = [this]() {
                           if (buttonRight.time > currentTime - BUTTON_DELAY) return;

                           buttonRight.pushed = true;
                           buttonRight.time   = currentTime;
                           scrollCounter      = 0;
                       };

    buttonRight.release = [this]() {
                              if (!buttonRight.pushed) return;

                              buttonRight.pushed = false;
                          };

    // === BUTTON A === //
    buttonA.push = [this]() {
                       if (!buttonA.pushed) { // first push
                           buttonA.pushed = true;
                           buttonA.time   = currentTime;
                           scrollCounter  = 0;
                       } else { // holding button
                           if ((currentTime - buttonA.time > 800) && !buttonA.hold) {
                               if (currentMenu->list->get(currentMenu->selected).hold) currentMenu->list->get(
                                       currentMenu->selected).hold();
                               buttonA.hold = true;
                           }
                       }
                   };

    buttonA.release = [this]() {
                          if (!buttonA.pushed) return;  // exit when button wasn't pushed before

                          if (!buttonA.hold && (currentTime - buttonA.time > 80)) {
                              switch (mode) {
                              case SCREEN_MODE_MENU:

                                  if (currentMenu->list->get(currentMenu->selected).click) {
                                      currentMenu->list->get(currentMenu->selected).click();
                                  }
                                  break;

                              case SCREEN_MODE_PACKETMONITOR:
                              case SCREEN_MODE_LOADSCAN:
                                  scan.stop();
                                  mode = SCREEN_MODE_MENU;
                                  break;
                              }
                          }

                          buttonA.pushed = false;
                          buttonA.hold   = false;
                      };

    // === BUTTON B === //
    buttonB.push = [this]() {
                       if (!buttonB.pushed && (buttonB.time > currentTime - BUTTON_DELAY)) return;

                       buttonB.pushed = true;
                       buttonB.time   = currentTime;
                       scrollCounter  = 0;
                   };

    buttonB.release = [this]() {
                          if (!buttonB.pushed) return;

                          switch (mode) {
                          case SCREEN_MODE_MENU:
                              goBack();
                              break;

                          case SCREEN_MODE_PACKETMONITOR:
                          case SCREEN_MODE_LOADSCAN:
                              scan.stop();
                              mode = SCREEN_MODE_MENU;
                              break;
                          }

                          buttonB.pushed = false;
                      };

    // === RUN SETUPS === //
    // setting pin modes for buttons
    if (buttonUp.enabled && buttonUp.setup) buttonUp.setup();

    if (buttonDown.enabled && buttonDown.setup) buttonDown.setup();

    if (buttonLeft.enabled && buttonLeft.setup) buttonLeft.setup();

    if (buttonRight.enabled && buttonRight.setup) buttonRight.setup();

    if (buttonA.enabled && buttonA.setup) buttonA.setup();

    if (buttonB.enabled && buttonB.setup) buttonB.setup();

    // ===== MENUS ===== //

    // MAIN MENU
    createMenu(&mainMenu, NULL, [this]() {
        addMenuNode(&mainMenu, D_SCAN, &scanMenu);          /// SCAN
        addMenuNode(&mainMenu, D_SHOW, &showMenu);          // SHOW
        addMenuNode(&mainMenu, D_ATTACK, &attackMenu);      // ATTACK
        addMenuNode(&mainMenu, D_PACKET_MONITOR, [this]() { // PACKET MONITOR
            scan.start(SCAN_MODE_SNIFFER, 0, SCAN_MODE_OFF, 0, false, wifi_channel);
            mode = SCREEN_MODE_PACKETMONITOR;
        });
    });

    // SCAN MENU
    createMenu(&scanMenu, &mainMenu, [this]() {
        addMenuNode(&scanMenu, D_SCAN_APST, [this]() { // SCAN AP + ST
            scan.start(SCAN_MODE_ALL, 15000, SCAN_MODE_OFF, 0, true, wifi_channel);
            mode = SCREEN_MODE_LOADSCAN;
        });
        addMenuNode(&scanMenu, D_SCAN_AP, [this]() { // SCAN AP
            scan.start(SCAN_MODE_APS, 0, SCAN_MODE_OFF, 0, true, wifi_channel);
            mode = SCREEN_MODE_LOADSCAN;
        });
        addMenuNode(&scanMenu, D_SCAN_ST, [this]() { // SCAN ST
            scan.start(SCAN_MODE_STATIONS, 30000, SCAN_MODE_OFF, 0, true, wifi_channel);
            mode = SCREEN_MODE_LOADSCAN;
        });
    });

    // SHOW MENU
    createMenu(&showMenu, &mainMenu, [this]() {
        addMenuNode(&showMenu, []() { // Accesspoints 0 [0]
            return buildString(str(D_ACCESSPOINTS), (String)accesspoints.count(), CHARS_PER_LINE);
        }, &apListMenu);
        addMenuNode(&showMenu, []() { // Stations 0 [0]
            return buildString(str(D_STATIONS), (String)stations.count(), CHARS_PER_LINE);
        }, &stationListMenu);
        addMenuNode(&showMenu, []() { // Names 0 [0]
            return buildString(str(D_NAMES), (String)names.count(), CHARS_PER_LINE);
        }, &nameListMenu);
        addMenuNode(&showMenu, []() { // SSIDs 0
            return buildString(str(D_SSIDS), (String)ssids.count(), CHARS_PER_LINE);
        }, &ssidListMenu);
    });

    // AP LIST MENU
    createMenu(&apListMenu, &showMenu, [this]() {
        // add APs to list
        int c = accesspoints.count();

        for (int i = 0; i < c; i++) {
            addMenuNode(&apListMenu, [i]() {
                return b2a(accesspoints.getSelected(i)) + accesspoints.getSSID(i);
            }, [this, i]() {
                accesspoints.getSelected(i) ? accesspoints.deselect(i) : accesspoints.select(i);
            }, [this, i]() {
                selectedID = i;
                changeMenu(&apMenu);
            });
        }
        addMenuNode(&apListMenu, D_SELECT_ALL, [this]() { // SELECT ALL
            accesspoints.selectAll();
            changeMenu(&apListMenu);
        });
        addMenuNode(&apListMenu, D_DESELECT_ALL, [this]() { // DESELECT ALL
            accesspoints.deselectAll();
            changeMenu(&apListMenu);
        });
        addMenuNode(&apListMenu, D_REMOVE_ALL, [this]() { // REMOVE ALL
            accesspoints.removeAll();
            goBack();
        });
    });

    // STATION LIST MENU
    createMenu(&stationListMenu, &showMenu, [this]() {
        // add stations to list
        int c = stations.count();

        for (int i = 0; i < c; i++) {
            addMenuNode(&stationListMenu, [i]() {
                return b2a(stations.getSelected(i)) +
                (stations.hasName(i) ? stations.getNameStr(i) : stations.getMacVendorStr(i));
            }, [this, i]() {
                stations.getSelected(i) ? stations.deselect(i) : stations.select(i);
            }, [this, i]() {
                selectedID = i;
                changeMenu(&stationMenu);
            });
        }

        addMenuNode(&stationListMenu, D_SELECT_ALL, [this]() { // SELECT ALL
            stations.selectAll();
            changeMenu(&stationListMenu);
        });
        addMenuNode(&stationListMenu, D_DESELECT_ALL, [this]() { // DESELECT ALL
            stations.deselectAll();
            changeMenu(&stationListMenu);
        });
        addMenuNode(&stationListMenu, D_REMOVE_ALL, [this]() { // REMOVE ALL
            stations.removeAll();
            goBack();
        });
    });

    // NAME LIST MENU
    createMenu(&nameListMenu, &showMenu, [this]() {
        // add device names to list
        int c = names.count();

        for (int i = 0; i < c; i++) {
            addMenuNode(&nameListMenu, [i]() {
                return names.getSelectedStr(i) + names.getName(i);
            }, [this, i]() {
                names.getSelected(i) ? names.deselect(i) : names.select(i);
            }, [this, i]() {
                selectedID = i;
                changeMenu(&nameMenu);
            });
        }
        addMenuNode(&nameListMenu, D_SELECT_ALL, [this]() { // SELECT ALL
            names.selectAll();
            changeMenu(&nameListMenu);
        });
        addMenuNode(&nameListMenu, D_DESELECT_ALL, [this]() { // DESELECT ALL
            names.deselectAll();
            changeMenu(&nameListMenu);
        });
        addMenuNode(&nameListMenu, D_REMOVE_ALL, [this]() { // REMOVE ALL
            names.removeAll();
            goBack();
        });
    });

    // SSID LIST MENU
    createMenu(&ssidListMenu, &showMenu, [this]() {
        addMenuNode(&ssidListMenu, D_CLONE_APS, [this]() { // CLONE APs
            ssids.cloneSelected(true);
            changeMenu(&ssidListMenu);
            ssids.save(false);
        });
        addMenuNode(&ssidListMenu, [this]() {
            return b2a(ssids.getRandom()) + str(D_RANDOM_MODE); // *RANDOM MODE
        }, [this]() {
            if (ssids.getRandom()) ssids.disableRandom();
            else ssids.enableRandom(10);
            changeMenu(&ssidListMenu);
        });

        // add ssids to list
        int c = ssids.count();

        for (int i = 0; i < c; i++) {
            addMenuNode(&ssidListMenu, [i]() {
                return ssids.getName(i).substring(0, ssids.getLen(i));
            }, [this, i]() {
                selectedID = i;
                changeMenu(&ssidMenu);
            }, [this, i]() {
                ssids.remove(i);
                changeMenu(&ssidListMenu);
                ssidListMenu.selected = i;
            });
        }

        addMenuNode(&ssidListMenu, D_REMOVE_ALL, [this]() { // REMOVE ALL
            ssids.removeAll();
            goBack();
        });
    });

    // AP MENU
    createMenu(&apMenu, &apListMenu, [this]() {
        addMenuNode(&apMenu, [this]() {
            return accesspoints.getSelectedStr(selectedID)  + accesspoints.getSSID(selectedID); // *<ssid>
        }, [this]() {
            accesspoints.getSelected(selectedID) ? accesspoints.deselect(selectedID) : accesspoints.select(selectedID);
        });
        addMenuNode(&apMenu, [this]() {
            return str(D_ENCRYPTION) + accesspoints.getEncStr(selectedID);
        }, NULL);                                                                          // Encryption: -/WPA2
        addMenuNode(&apMenu, [this]() {
            return str(D_RSSI) + (String)accesspoints.getRSSI(selectedID);
        }, NULL);                                                                          // RSSI: -90
        addMenuNode(&apMenu, [this]() {
            return str(D_CHANNEL) + (String)accesspoints.getCh(selectedID);
        }, NULL);                                                                          // Channel: 11
        addMenuNode(&apMenu, [this]() {
            return accesspoints.getMacStr(selectedID);
        }, NULL);                                                                          // 00:11:22:00:11:22
        addMenuNode(&apMenu, [this]() {
            return str(D_VENDOR) + accesspoints.getVendorStr(selectedID);
        }, NULL);                                                                          // Vendor: INTEL
        addMenuNode(&apMenu, [this]() {
            return accesspoints.getSelected(selectedID) ? str(D_DESELECT) : str(D_SELECT); // SELECT/DESELECT
        }, [this]() {
            accesspoints.getSelected(selectedID) ? accesspoints.deselect(selectedID) : accesspoints.select(selectedID);
        });
        addMenuNode(&apMenu, D_CLONE, [this]() { // CLONE
            ssids.add(accesspoints.getSSID(selectedID), accesspoints.getEnc(selectedID) != ENC_TYPE_NONE, 60, true);
            changeMenu(&showMenu);
            ssids.save(false);
        });
        addMenuNode(&apMenu, D_REMOVE, [this]() { // REMOVE
            accesspoints.remove(selectedID);
            apListMenu.list->remove(apListMenu.selected);
            goBack();
        });
    });

    // STATION MENU
    createMenu(&stationMenu, &stationListMenu, [this]() {
        addMenuNode(&stationMenu, [this]() {
            return stations.getSelectedStr(selectedID) +
            (stations.hasName(selectedID) ? stations.getNameStr(selectedID) : stations.getMacVendorStr(selectedID)); // <station
                                                                                                                     // name>
        }, [this]() {
            stations.getSelected(selectedID) ? stations.deselect(selectedID) : stations.select(selectedID);
        });
        addMenuNode(&stationMenu, [this]() {
            return stations.getMacStr(selectedID);
        }, NULL);                                             // 00:11:22:00:11:22
        addMenuNode(&stationMenu, [this]() {
            return str(D_VENDOR) + stations.getVendorStr(selectedID);
        }, NULL);                                             // Vendor: INTEL
        addMenuNode(&stationMenu, [this]() {
            return str(D_AP) + stations.getAPStr(selectedID); // AP: someAP
        }, [this]() {
            int apID = accesspoints.find(stations.getAP(selectedID));

            if (apID >= 0) {
                selectedID = apID;
                changeMenu(&apMenu);
            }
        });
        addMenuNode(&stationMenu, [this]() {
            return str(D_PKTS) + String(*stations.getPkts(selectedID));
        }, NULL);                                                                      // Pkts: 12
        addMenuNode(&stationMenu, [this]() {
            return str(D_CHANNEL) + String(stations.getCh(selectedID));
        }, NULL);                                                                      // Channel: 11
        addMenuNode(&stationMenu, [this]() {
            return str(D_SEEN) + stations.getTimeStr(selectedID);
        }, NULL);                                                                      // Seen: <1min

        addMenuNode(&stationMenu, [this]() {
            return stations.getSelected(selectedID) ? str(D_DESELECT) : str(D_SELECT); // SELECT/DESELECT
        }, [this]() {
            stations.getSelected(selectedID) ? stations.deselect(selectedID) : stations.select(selectedID);
        });
        addMenuNode(&stationMenu, D_REMOVE, [this]() { // REMOVE
            stations.remove(selectedID);
            stationListMenu.list->remove(stationListMenu.selected);
            goBack();
        });
    });

    // NAME MENU
    createMenu(&nameMenu, &nameListMenu, [this]() {
        addMenuNode(&nameMenu, [this]() {
            return names.getSelectedStr(selectedID) + names.getName(selectedID); // <station name>
        }, [this]() {
            names.getSelected(selectedID) ? names.deselect(selectedID) : names.select(selectedID);
        });
        addMenuNode(&nameMenu, [this]() {
            return names.getMacStr(selectedID);
        }, NULL);                                                                   // 00:11:22:00:11:22
        addMenuNode(&nameMenu, [this]() {
            return str(D_VENDOR) + names.getVendorStr(selectedID);
        }, NULL);                                                                   // Vendor: INTEL
        addMenuNode(&nameMenu, [this]() {
            return str(D_AP) + names.getBssidStr(selectedID);
        }, NULL);                                                                   // AP: 00:11:22:00:11:22
        addMenuNode(&nameMenu, [this]() {
            return str(D_CHANNEL) + (String)names.getCh(selectedID);
        }, NULL);                                                                   // Channel: 11

        addMenuNode(&nameMenu, [this]() {
            return names.getSelected(selectedID) ? str(D_DESELECT) : str(D_SELECT); // SELECT/DESELECT
        }, [this]() {
            names.getSelected(selectedID) ? names.deselect(selectedID) : names.select(selectedID);
        });
        addMenuNode(&nameMenu, D_REMOVE, [this]() { // REMOVE
            names.remove(selectedID);
            nameListMenu.list->remove(nameListMenu.selected);
            goBack();
        });
    });

    // SSID MENU
    createMenu(&ssidMenu, &ssidListMenu, [this]() {
        addMenuNode(&ssidMenu, [this]() {
            return ssids.getName(selectedID).substring(0, ssids.getLen(selectedID));
        }, NULL);                                                   // SSID
        addMenuNode(&ssidMenu, [this]() {
            return str(D_ENCRYPTION) + ssids.getEncStr(selectedID); // WPA2
        }, [this]() {
            ssids.setWPA2(selectedID, !ssids.getWPA2(selectedID));
        });
        addMenuNode(&ssidMenu, D_REMOVE, [this]() { // REMOVE
            ssids.remove(selectedID);
            ssidListMenu.list->remove(ssidListMenu.selected);
            goBack();
        });
    });

    // ATTACK MENU
    createMenu(&attackMenu, &mainMenu, [this]() {
        addMenuNode(&attackMenu, [this]() { // *DEAUTH 0/0
            if (attack.isRunning()) return buildString(b2a(deauthSelected) + str(D_DEAUTH),
                                                       (String)attack.getDeauthPkts() + SLASH +
                                                       (String)attack.getDeauthMaxPkts(), CHARS_PER_LINE);
            else return buildString(b2a(deauthSelected) + str(D_DEAUTH), (String)scan.countSelected(), CHARS_PER_LINE);
        }, [this]() { // deauth
            deauthSelected = !deauthSelected;

            if (attack.isRunning()) {
                attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                             settings.getAttackTimeout() * 1000);
            }
        });
        addMenuNode(&attackMenu, [this]() { // *BEACON 0/0
            if (attack.isRunning()) return buildString(b2a(beaconSelected) + str(D_BEACON),
                                                       (String)attack.getBeaconPkts() + SLASH +
                                                       (String)attack.getBeaconMaxPkts(), CHARS_PER_LINE);
            else return buildString(b2a(beaconSelected) + str(D_BEACON), (String)ssids.count(), CHARS_PER_LINE);
        }, [this]() { // beacon
            beaconSelected = !beaconSelected;

            if (attack.isRunning()) {
                attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                             settings.getAttackTimeout() * 1000);
            }
        });
        addMenuNode(&attackMenu, [this]() { // *PROBE 0/0
            if (attack.isRunning()) return buildString(b2a(probeSelected) + str(D_PROBE),
                                                       (String)attack.getProbePkts() + SLASH +
                                                       (String)attack.getProbeMaxPkts(), CHARS_PER_LINE);
            else return buildString(b2a(probeSelected) + str(D_PROBE), (String)ssids.count(), CHARS_PER_LINE);
        }, [this]() { // probe
            probeSelected = !probeSelected;

            if (attack.isRunning()) {
                attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                             settings.getAttackTimeout() * 1000);
            }
        });
        addMenuNode(&attackMenu, []() { // START
            return buildString(str(attack.isRunning() ? D_STOP_ATTACK : D_START_ATTACK),
                               attack.getPacketRate() > 0 ? (String)attack.getPacketRate() : String(), CHARS_PER_LINE);
        }, [this]() {
            if (attack.isRunning()) attack.stop();
            else attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                              settings.getAttackTimeout() * 1000);
        });
    });

    // ===================== //

    // set current menu to main menu
    changeMenu(&mainMenu);
    enabled   = true;
    startTime = currentTime;
}

void DisplayUI::update() {
    if (!enabled) return;

    // when display is off
    if (mode == SCREEN_MODE_OFF) {
        if (updateButton(&buttonA)) {
            on();
            buttonA.hold = true; // to make sure you don't double click
        }
    }

    else {
        // timeout to save display life
        if ((mode == SCREEN_MODE_MENU) && (settings.getDisplayTimeout() > 0) &&
            (currentTime > settings.getDisplayTimeout() * 1000)) {
            uint32_t buttonTimeout = currentTime - settings.getDisplayTimeout() * 1000;

            if ((buttonUp.time < buttonTimeout)
                && (buttonDown.time < buttonTimeout)
                && (buttonLeft.time < buttonTimeout)
                && (buttonRight.time < buttonTimeout)
                && (buttonA.time < buttonTimeout)
                && (buttonB.time < buttonTimeout)) {
                off();
            }
        }

        // only one button can be pressed at a time
        if (updateButton(&buttonB)) draw();
        else if (updateButton(&buttonA)) draw();
        else if (updateButton(&buttonUp)) draw();
        else if (updateButton(&buttonDown)) draw();
        else if (updateButton(&buttonLeft)) draw();
        else if (updateButton(&buttonRight)) draw();
        else draw();
    }
}

void DisplayUI::draw() {
    if ((currentTime - drawTime > DRAW_INTERVAL) && currentMenu) {
        drawTime = currentTime;

        // ===== adjustable ===== //
        display.clear();                           // clear display
        display.setTextAlignment(TEXT_ALIGN_LEFT); // reset text alignment just in case ;)
        // ====================== //

        switch (mode) {
        case SCREEN_MODE_BUTTON_TEST:
            drawButtonTest();
            break;

        case SCREEN_MODE_MENU:
            drawMenu();
            break;

        case SCREEN_MODE_LOADSCAN:
            drawLoadingScan();
            break;

        case SCREEN_MODE_PACKETMONITOR:
            drawPacketMonitor();
            break;

        case SCREEN_MODE_INTRO:

            if (currentTime - startTime >= SCREEN_INTRO_TIME) {
                mode = SCREEN_MODE_MENU;
            }
            drawIntro();
            break;
        }

        // ===== adjustable ===== //
        display.display(); // draw changes
        // ====================== //
    }
}

void DisplayUI::drawString(int x, int y, String str) {
    // ===== adjustable ===== //
    display.drawString(x, y, replaceUtf8(str, String(QUESTIONMARK)));
    // ====================== //
}

void DisplayUI::drawButtonTest() {
    // ===== adjustable ===== //
    if (buttonUp.enabled) display.drawString(0, 0, str(D_UP) + b2s(buttonUp.pushed));

    if (buttonDown.enabled) display.drawString(0, 9, str(D_DOWN) + b2s(buttonDown.pushed));

    if (buttonLeft.enabled) display.drawString(0, 18, str(D_LEFT) + b2s(buttonLeft.pushed));

    if (buttonRight.enabled) display.drawString(0, 27, str(D_RIGHT) + b2s(buttonRight.pushed));

    if (buttonA.enabled) display.drawString(0, 36, str(D_A) + b2s(buttonA.pushed));

    if (buttonB.enabled) display.drawString(0, 45, str(D_B) + b2s(buttonB.pushed));
    // ====================== //
}

void DisplayUI::drawMenu() {
    String tmp;
    int    tmpLen;
    int    row = (currentMenu->selected / 5) * 5;

    // correct selected if it's off
    if (currentMenu->selected < 0) currentMenu->selected = 0;
    else if (currentMenu->selected >= currentMenu->list->size()) currentMenu->selected = currentMenu->list->size() - 1;

    // draw menu entries
    for (int i = row; i < currentMenu->list->size() && i < row + 5; i++) {
        tmp    = currentMenu->list->get(i).getStr();
        tmpLen = tmp.length();

        // horizontal scrolling
        if ((currentMenu->selected == i) && (tmpLen > CHARS_PER_LINE)) {
            tmp = tmp.substring(scrollCounter / SCROLL_SPEED);
            scrollCounter++;

            if (scrollCounter / SCROLL_SPEED > tmpLen - CHARS_PER_LINE) scrollCounter = 0;
        }

        tmp = (currentMenu->selected == i ? CURSOR : SPACE) + tmp;
        drawString(0, (i - row) * 12, tmp);
    }
}

void DisplayUI::drawLoadingScan() {
    // ===== adjustable ===== //
    if (scan.isScanning()) {
        if (scan.isSniffing()) { // Scanning for Stations
            display.drawString(2, 0,
                               buildString(str(D_LOADING_SCREEN_0), (String)scan.getPercentage() + PERCENT,
                                           CHARS_PER_LINE));
            display.drawProgressBar(5, 22, 118, 8, scan.getPercentage());
            display.drawString(2, 36, str(D_LOADING_SCREEN_1) + (String)accesspoints.count());
            display.drawString(2, 50, str(D_LOADING_SCREEN_2) + (String)stations.count());
        } else { // Scanning for APs
            display.drawString(2, 10, str(D_LOADING_SCREEN_3));
            display.drawString(2, 24, str(D_LOADING_SCREEN_4));
        }
    } else {
        mode = SCREEN_MODE_MENU;
        changeMenu(&showMenu);
    }
    // ====================== //
}

void DisplayUI::drawPacketMonitor() {
    // ===== adjustable ===== //
    double scale = scan.getScaleFactor(50);

    display.drawString(0, 0, str(D_CH) + String(wifi_channel));
    display.drawString(40, 0, str(D_PKTS) + String(scan.getPacketRate()));
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 0, String(scan.deauths));
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    if (scan.getMaxPacket() > 0) {
        for (int i = 0; i < SCAN_PACKET_LIST_SIZE * 2; i += 2) {
            display.drawLine(i, 64, i, 64 - scan.getPackets(i / 2) * scale);
            display.drawLine(i + 1, 64, i + 1, 64 - scan.getPackets(i / 2) * scale);
        }
    }
    // ====================== //
}

void DisplayUI::drawIntro() {
    // ===== adjustable ===== //
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 6, String(F("ESP8266 Deauther")));
    display.drawString(64, 20, String(F("by @Spacehuhn")));
    display.drawString(64, 34, String(F("")));
    display.drawString(64, 48, settings.getVersion());
    // ====================== //
}

bool DisplayUI::updateButton(Button* button) {
    // direct exit when button is disabled or has no read function
    if (!button->enabled || !button->read) return false;

    // read pin
    if (button->read()) {
        if (button->push) button->push();
    } else {
        if (button->release) button->release();
    }

    return button->pushed;
}

void DisplayUI::clearMenu(Menu* menu) {
    while (menu->list->size() > 0) {
        menu->list->remove(0);
    }
}

void DisplayUI::changeMenu(Menu* menu) {
    if (menu) {
        // only open list menu if it has nodes
        if (((menu == &apListMenu) && (accesspoints.count() == 0)) ||
            ((menu == &stationListMenu) && (stations.count() == 0)) ||
            ((menu == &nameListMenu) && (names.count() == 0))) {
            return;
        }

        if (currentMenu) clearMenu(currentMenu);
        currentMenu           = menu;
        currentMenu->selected = 0;
        buttonA.time          = currentTime;

        if (selectedID < 0) selectedID = 0;

        if (currentMenu->parentMenu) {
            addMenuNode(currentMenu, D_BACK, currentMenu->parentMenu); // add [BACK]
            currentMenu->selected = 1;
        }

        if (currentMenu->build) currentMenu->build();
    }
}

void DisplayUI::goBack() {
    if (currentMenu->parentMenu) changeMenu(currentMenu->parentMenu);
}

void DisplayUI::createMenu(Menu* menu, Menu* parent, std::function<void()>build) {
    menu->list       = new SimpleList<MenuNode>;
    menu->parentMenu = parent;
    menu->selected   = 0;
    menu->build      = build;
}

void DisplayUI::addMenuNode(Menu* menu, std::function<String()>getStr, std::function<void()>click,
                            std::function<void()>hold) {
    menu->list->add(MenuNode{ getStr, click, hold });
}

void DisplayUI::addMenuNode(Menu* menu, std::function<String()>getStr, std::function<void()>click) {
    addMenuNode(menu, getStr, click, NULL);
}

void DisplayUI::addMenuNode(Menu* menu, std::function<String()>getStr, Menu* next) {
    addMenuNode(menu, getStr, [this, next]() {
        changeMenu(next);
    });
}

void DisplayUI::addMenuNode(Menu* menu, const char* ptr, std::function<void()>click) {
    addMenuNode(menu, [ptr]() {
        return str(ptr);
    }, click);
}

void DisplayUI::addMenuNode(Menu* menu, const char* ptr, Menu* next) {
    addMenuNode(menu, [ptr]() {
        return str(ptr);
    }, next);
}