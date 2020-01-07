#include "DisplayUI.h"

// ===== adjustable ===== //
void DisplayUI::configInit() {
    // initialize display
    display.init();

    /*
       In case of a compiler (conversion char/uint8_t) error,
       make sure to have version 4 of the display library installed
       https://github.com/ThingPulse/esp8266-oled-ssd1306/releases/tag/4.0.0
     */
    display.setFont(DejaVu_Sans_Mono_12);

    display.setContrast(255);

    if (FLIP_DIPLAY) display.flipScreenVertically();

    display.clear();
    display.display();
}

void DisplayUI::configOn() {
    display.displayOn();
}

void DisplayUI::configOff() {
    display.displayOff();
}

void DisplayUI::updatePrefix() {
    display.clear();
}

void DisplayUI::updateSuffix() {
    display.display();
}

void DisplayUI::drawString(int x, int y, String str) {
    display.drawString(x, y, replaceUtf8(str, String(QUESTIONMARK)));
}

void DisplayUI::drawString(int row, String str) {
    drawString(0, row * lineHeight, str);
}

void DisplayUI::drawLine(int x1, int y1, int x2, int y2) {
    display.drawLine(x1, y1, x2, y2);
}

// ====================== //


DisplayUI::DisplayUI() {}

DisplayUI::~DisplayUI() {}


void DisplayUI::setup() {
    configInit();
    setupButtons();
    buttonTime = currentTime;

    clockHour   = random(12);
    clockMinute = random(60);

    // ===== MENUS ===== //

    // MAIN MENU
    createMenu(&mainMenu, NULL, [this]() {
        addMenuNode(&mainMenu, D_SCAN, &scanMenu);          /// SCAN
        addMenuNode(&mainMenu, D_SHOW, &showMenu);          // SHOW
        addMenuNode(&mainMenu, D_ATTACK, &attackMenu);      // ATTACK
        addMenuNode(&mainMenu, D_PACKET_MONITOR, [this]() { // PACKET MONITOR
            scan.start(SCAN_MODE_SNIFFER, 0, SCAN_MODE_OFF, 0, false, wifi_channel);
            mode = DISPLAY_MODE::PACKETMONITOR;
        });

        addMenuNode(&mainMenu, D_CLOCK, [this]() { // PACKET MONITOR
            mode = DISPLAY_MODE::CLOCK;
            display.setFont(ArialMT_Plain_24);
            display.setTextAlignment(TEXT_ALIGN_CENTER);
        });

#ifdef HIGHLIGHT_LED
        addMenuNode(&mainMenu, D_LED, [this]() { // LED
            highlightLED = !highlightLED;
            digitalWrite(HIGHLIGHT_LED, highlightLED);
        });
#endif // ifdef HIGHLIGHT_LED
    });

    // SCAN MENU
    createMenu(&scanMenu, &mainMenu, [this]() {
        addMenuNode(&scanMenu, D_SCAN_APST, [this]() { // SCAN AP + ST
            scan.start(SCAN_MODE_ALL, 15000, SCAN_MODE_OFF, 0, true, wifi_channel);
            mode = DISPLAY_MODE::LOADSCAN;
        });
        addMenuNode(&scanMenu, D_SCAN_AP, [this]() { // SCAN AP
            scan.start(SCAN_MODE_APS, 0, SCAN_MODE_OFF, 0, true, wifi_channel);
            mode = DISPLAY_MODE::LOADSCAN;
        });
        addMenuNode(&scanMenu, D_SCAN_ST, [this]() { // SCAN ST
            scan.start(SCAN_MODE_STATIONS, 30000, SCAN_MODE_OFF, 0, true, wifi_channel);
            mode = DISPLAY_MODE::LOADSCAN;
        });
    });

    // SHOW MENU
    createMenu(&showMenu, &mainMenu, [this]() {
        addMenuNode(&showMenu, [this]() { // Accesspoints 0 [0]
            return leftRight(str(D_ACCESSPOINTS), (String)accesspoints.count(), maxLen - 1);
        }, &apListMenu);
        addMenuNode(&showMenu, [this]() { // Stations 0 [0]
            return leftRight(str(D_STATIONS), (String)stations.count(), maxLen - 1);
        }, &stationListMenu);
        addMenuNode(&showMenu, [this]() { // Names 0 [0]
            return leftRight(str(D_NAMES), (String)names.count(), maxLen - 1);
        }, &nameListMenu);
        addMenuNode(&showMenu, [this]() { // SSIDs 0
            return leftRight(str(D_SSIDS), (String)ssids.count(), maxLen - 1);
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
            if (attack.isRunning()) return leftRight(b2a(deauthSelected) + str(D_DEAUTH),
                                                     (String)attack.getDeauthPkts() + SLASH +
                                                     (String)attack.getDeauthMaxPkts(), maxLen - 1);
            else return leftRight(b2a(deauthSelected) + str(D_DEAUTH), (String)scan.countSelected(), maxLen - 1);
        }, [this]() { // deauth
            deauthSelected = !deauthSelected;

            if (attack.isRunning()) {
                attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                             settings.getAttackSettings().timeout * 1000);
            }
        });
        addMenuNode(&attackMenu, [this]() { // *BEACON 0/0
            if (attack.isRunning()) return leftRight(b2a(beaconSelected) + str(D_BEACON),
                                                     (String)attack.getBeaconPkts() + SLASH +
                                                     (String)attack.getBeaconMaxPkts(), maxLen - 1);
            else return leftRight(b2a(beaconSelected) + str(D_BEACON), (String)ssids.count(), maxLen - 1);
        }, [this]() { // beacon
            beaconSelected = !beaconSelected;

            if (attack.isRunning()) {
                attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                             settings.getAttackSettings().timeout * 1000);
            }
        });
        addMenuNode(&attackMenu, [this]() { // *PROBE 0/0
            if (attack.isRunning()) return leftRight(b2a(probeSelected) + str(D_PROBE),
                                                     (String)attack.getProbePkts() + SLASH +
                                                     (String)attack.getProbeMaxPkts(), maxLen - 1);
            else return leftRight(b2a(probeSelected) + str(D_PROBE), (String)ssids.count(), maxLen - 1);
        }, [this]() { // probe
            probeSelected = !probeSelected;

            if (attack.isRunning()) {
                attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                             settings.getAttackSettings().timeout * 1000);
            }
        });
        addMenuNode(&attackMenu, [this]() { // START
            return leftRight(str(attack.isRunning() ? D_STOP_ATTACK : D_START_ATTACK),
                             attack.getPacketRate() > 0 ? (String)attack.getPacketRate() : String(), maxLen - 1);
        }, [this]() {
            if (attack.isRunning()) attack.stop();
            else attack.start(beaconSelected, deauthSelected, false, probeSelected, true,
                              settings.getAttackSettings().timeout * 1000);
        });
    });

    // ===================== //

    // set current menu to main menu
    changeMenu(&mainMenu);
    enabled   = true;
    startTime = currentTime;
}

#ifdef HIGHLIGHT_LED
void DisplayUI::setupLED() {
    pinMode(HIGHLIGHT_LED, OUTPUT);
    digitalWrite(HIGHLIGHT_LED, HIGH);
    highlightLED = true;
}

#endif // ifdef HIGHLIGHT_LED

void DisplayUI::update() {
    if (!enabled) return;

    up->update();
    down->update();
    a->update();
    b->update();

    draw();

    uint32_t timeout = settings.getDisplaySettings().timeout * 1000;

    if (currentTime > timeout) {
        if (!tempOff) {
            if (buttonTime < currentTime - timeout) off();
        } else {
            if (buttonTime > currentTime - timeout) on();
        }
    }
}

void DisplayUI::on() {
    if (enabled) {
        configOn();
        tempOff    = false;
        buttonTime = currentTime; // update a button time to keep display on
        prntln(D_MSG_DISPLAY_ON);
    } else {
        prntln(D_ERROR_NOT_ENABLED);
    }
}

void DisplayUI::off() {
    if (enabled) {
        configOff();
        tempOff = true;
        prntln(D_MSG_DISPLAY_OFF);
    } else {
        prntln(D_ERROR_NOT_ENABLED);
    }
}

void DisplayUI::setupButtons() {
    up   = new ButtonPullup(BUTTON_UP);
    down = new ButtonPullup(BUTTON_DOWN);
    a    = new ButtonPullup(BUTTON_A);
    b    = new ButtonPullup(BUTTON_B);

    // === BUTTON UP === //
    up->setOnClicked([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;

        if (!tempOff) {
            if (mode == DISPLAY_MODE::MENU) {                 // when in menu, go up or down with cursor
                if (currentMenu->selected > 0) currentMenu->selected--;
                else currentMenu->selected = currentMenu->list->size() - 1;
            } else if (mode == DISPLAY_MODE::PACKETMONITOR) { // when in packet monitor, change channel
                scan.setChannel(wifi_channel + 1);
            } else if (mode == DISPLAY_MODE::CLOCK) {         // when in packet monitor, change channel
                setTime(clockHour, clockMinute + 1, clockSecond);
            }
        }
    });

    up->setOnHolding([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;
        if (!tempOff) {
            if (mode == DISPLAY_MODE::MENU) {                 // when in menu, go up or down with cursor
                if (currentMenu->selected > 0) currentMenu->selected--;
                else currentMenu->selected = currentMenu->list->size() - 1;
            } else if (mode == DISPLAY_MODE::PACKETMONITOR) { // when in packet monitor, change channel
                scan.setChannel(wifi_channel + 1);
            } else if (mode == DISPLAY_MODE::CLOCK) {         // when in packet monitor, change channel
                setTime(clockHour, clockMinute + 10, clockSecond);
            }
        }
    }, buttonDelay);

    // === BUTTON DOWN === //
    down->setOnClicked([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;
        if (!tempOff) {
            if (mode == DISPLAY_MODE::MENU) {                 // when in menu, go up or down with cursor
                if (currentMenu->selected < currentMenu->list->size() - 1) currentMenu->selected++;
                else currentMenu->selected = 0;
            } else if (mode == DISPLAY_MODE::PACKETMONITOR) { // when in packet monitor, change channel
                scan.setChannel(wifi_channel - 1);
            } else if (mode == DISPLAY_MODE::CLOCK) {         // when in packet monitor, change channel
                setTime(clockHour, clockMinute - 1, clockSecond);
            }
        }
    });

    down->setOnHolding([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;
        if (!tempOff) {
            if (mode == DISPLAY_MODE::MENU) {                 // when in menu, go up or down with cursor
                if (currentMenu->selected < currentMenu->list->size() - 1) currentMenu->selected++;
                else currentMenu->selected = 0;
            } else if (mode == DISPLAY_MODE::PACKETMONITOR) { // when in packet monitor, change channel
                scan.setChannel(wifi_channel - 1);
            }

            else if (mode == DISPLAY_MODE::CLOCK) { // when in packet monitor, change channel
                setTime(clockHour, clockMinute - 10, clockSecond);
            }
        }
    }, buttonDelay);

    // === BUTTON A === //
    a->setOnClicked([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;
        if (!tempOff) {
            switch (mode) {
                case DISPLAY_MODE::MENU:

                    if (currentMenu->list->get(currentMenu->selected).click) {
                        currentMenu->list->get(currentMenu->selected).click();
                    }
                    break;

                case DISPLAY_MODE::PACKETMONITOR:
                case DISPLAY_MODE::LOADSCAN:
                    scan.stop();
                    mode = DISPLAY_MODE::MENU;
                    break;

                case DISPLAY_MODE::CLOCK:
                    mode = DISPLAY_MODE::MENU;
                    display.setFont(DejaVu_Sans_Mono_12);
                    display.setTextAlignment(TEXT_ALIGN_LEFT);
                    break;
            }
        }
    });

    a->setOnHolding([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;
        if (!tempOff) {
            if (mode == DISPLAY_MODE::MENU) {
                if (currentMenu->list->get(currentMenu->selected).hold) {
                    currentMenu->list->get(currentMenu->selected).hold();
                }
            }
        }
    }, 800);

    // === BUTTON B === //
    b->setOnClicked([this]() {
        scrollCounter = 0;
        scrollTime    = currentTime;
        buttonTime    = currentTime;
        if (!tempOff) {
            switch (mode) {
                case DISPLAY_MODE::MENU:
                    goBack();
                    break;

                case DISPLAY_MODE::PACKETMONITOR:
                case DISPLAY_MODE::LOADSCAN:
                    scan.stop();
                    mode = DISPLAY_MODE::MENU;
                    break;

                case DISPLAY_MODE::CLOCK:
                    mode = DISPLAY_MODE::MENU;
                    display.setFont(DejaVu_Sans_Mono_12);
                    display.setTextAlignment(TEXT_ALIGN_LEFT);
                    break;
            }
        }
    });
}

String DisplayUI::getChannel() {
    String ch = String(wifi_channel);

    if (ch.length() < 2) ch = ' ' + ch;
    return ch;
}

void DisplayUI::draw() {
    if ((currentTime - drawTime > drawInterval) && currentMenu) {
        drawTime = currentTime;

        updatePrefix();

        if (clockTime < currentTime - 1000) {
            setTime(clockHour, clockMinute++, clockSecond + 1);
            clockTime += 1000;
        }

        switch (mode) {
            case DISPLAY_MODE::BUTTON_TEST:
                drawButtonTest();
                break;

            case DISPLAY_MODE::MENU:
                drawMenu();
                break;

            case DISPLAY_MODE::LOADSCAN:
                drawLoadingScan();
                break;

            case DISPLAY_MODE::PACKETMONITOR:
                drawPacketMonitor();
                break;

            case DISPLAY_MODE::INTRO:
                if (currentTime - startTime >= screenIntroTime) {
                    mode = DISPLAY_MODE::MENU;
                }
                drawIntro();
                break;
            case DISPLAY_MODE::CLOCK:
                drawClock();
                break;
        }

        updateSuffix();
    }
}

void DisplayUI::drawButtonTest() {
    drawString(0, str(D_UP) + b2s(up->read()));
    drawString(1, str(D_DOWN) + b2s(down->read()));
    drawString(2, str(D_A) + b2s(a->read()));
    drawString(3, str(D_B) + b2s(b->read()));
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
        if ((currentMenu->selected == i) && (tmpLen >= maxLen)) {
            tmp = tmp + tmp;
            tmp = tmp.substring(scrollCounter, scrollCounter + maxLen - 1);

            if (((scrollCounter > 0) && (scrollTime < currentTime - scrollSpeed)) || ((scrollCounter == 0) && (scrollTime < currentTime - scrollSpeed * 4))) {
                scrollTime = currentTime;
                scrollCounter++;
            }

            if (scrollCounter > tmpLen) scrollCounter = 0;
        }

        tmp = (currentMenu->selected == i ? CURSOR : SPACE) + tmp;
        drawString(0, (i - row) * 12, tmp);
    }
}

void DisplayUI::drawLoadingScan() {
    String percentage;

    if (scan.isScanning()) {
        percentage = String(scan.getPercentage()) + '%';
    } else {
        percentage = str(DSP_SCAN_DONE);
    }

    drawString(0, leftRight(str(DSP_SCAN_FOR), scan.getMode(), maxLen));
    drawString(1, leftRight(str(DSP_APS), String(accesspoints.count()), maxLen));
    drawString(2, leftRight(str(DSP_STS), String(stations.count()), maxLen));
    drawString(3, leftRight(str(DSP_PKTS), String(scan.getPacketRate()) + str(DSP_S), maxLen));
    drawString(4, center(percentage, maxLen));
}

void DisplayUI::drawPacketMonitor() {
    double scale = scan.getScaleFactor(sreenHeight - lineHeight - 2);

    String headline = leftRight(str(D_CH) + getChannel() + String(' ') + String('[') + String(scan.deauths) + String(']'), String(scan.getPacketRate()) + str(D_PKTS), maxLen);

    drawString(0, 0, headline);

    if (scan.getMaxPacket() > 0) {
        int i = 0;
        int x = 0;
        int y = 0;

        while (i < SCAN_PACKET_LIST_SIZE && x < screenWidth) {
            y = (sreenHeight-1) - (scan.getPackets(i) * scale);
            i++;

            // Serial.printf("%d,%d -> %d,%d\n", x, (sreenHeight-1), x, y);
            drawLine(x, (sreenHeight-1), x, y);
            x++;

            // Serial.printf("%d,%d -> %d,%d\n", x, (sreenHeight-1), x, y);
            drawLine(x, (sreenHeight-1), x, y);
            x++;
        }
        // Serial.println("---------");
    }
}

void DisplayUI::drawIntro() {
    drawString(0, center(str(D_INTRO_0), maxLen));
    drawString(1, center(str(D_INTRO_1), maxLen));
    drawString(2, center(str(D_INTRO_2), maxLen));
    drawString(3, center(str(D_INTRO_3), maxLen));
    drawString(4, center(DEAUTHER_VERSION, maxLen));
}

void DisplayUI::drawClock() {
    String clockTime = String(clockHour);

    clockTime += ':';
    if (clockMinute < 10) clockTime += '0';
    clockTime += String(clockMinute);

    display.drawString(64, 20, clockTime);
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
        buttonTime            = currentTime;

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

void DisplayUI::setTime(int h, int m, int s) {
    if (s >= 60) {
        s = 0;
        m++;
    }

    if (m >= 60) {
        m = 0;
        h++;
    }

    if (h >= 24) {
        h = 0;
    }

    if (s < 0) {
        s = 59;
        m--;
    }

    if (m < 0) {
        m = 59;
        h--;
    }

    if (h < 0) {
        h = 23;
    }

    clockHour   = h;
    clockMinute = m;
    clockSecond = s;
}
