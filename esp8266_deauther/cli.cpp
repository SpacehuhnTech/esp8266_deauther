/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "cli.h"

#include <SimpleCLI.h> // SimpleCLI library

#include "debug.h"     // debug(), debugln(), debugf()
#include "scan.h"
#include "strh.h"
#include "StringList.h"
#include "mac.h"
#include "vendor.h"
#include "attack.h"

// ram usage
extern "C" {
  #include "user_interface.h"
}

#define CLI_READ_RES()\
    res = read_and_wait();\
    if (res == "exit") {\
        debugln("Ok byeee");\
        return;\
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

        Command cmd_start = cli.addCommand("start", [](cmd* c) {
            String res;
            String cmd;

            debugln("Good morning friend.");

            // Command
            do {
                debugln("What can I do for you today?\n"
                        "Remember that you can always escape by typing 'exit'\n"
                        "  scan: Search for WiFi networks and clients\n"
                        "  beacon: Send WiFi network advertisement beacons\n"
                        "  deauth: Disrupt WiFi connections"
                        );
                CLI_READ_RES();
            } while (!(res == "scan" || res == "beacon" || res == "deauth"));

            if (res == "scan") {
                // Scan mode
                do {
                    debugln("Scan mode\n"
                            "  ap: Access Points (WiFi networks)\n"
                            "  st: Stations (WiFi clients)\n"
                            "  ap+st: Access Points and Stations");
                    CLI_READ_RES();
                } while (!(res == "ap" || res == "st" || res == "ap+st"));

                cmd += "scan -m " + res;

                // Scan time and channel(s)
                if (res != "ap") {
// Scan time
                    do {
                        debugln("Scan time\n"
                                "  >1: Station scan time in seconds");
                        CLI_READ_RES();
                    } while (!(res.toInt() > 0));
                    cmd += " -t " + res;

// Scan on channel(s)
                    debugln("Scan on channel(s)\n"
                            "  1-14: WiFi channel(s) to search on (for example: 1,6,11)");
                    CLI_READ_RES();
                    cmd += " -ch " + res;
                }

                // Retain scan results
                do {
                    debugln("Retain previous scan results\n"
                            "  y: Yes\n"
                            "  n: No");
                    CLI_READ_RES();
                } while (!(res == String('y') || res == String('n')));

                if (res == String('y')) {
                    cmd += " -r";
                }
            } else if (res == "beacon") {
                // SSIDs
                debugln("SSIDs (network names) for example \"network A\",\"network B\"\n");
                CLI_READ_RES();
                cmd += "beacon -s " + res;

                // From
                do {
                    debugln("Transmitter and sender MAC address\n"
                            "  for example '00:20:91:aa:bb:5cc\n"
                            "  random: generate random MAC address");
                    CLI_READ_RES();
                } while (!(res.length() == 17 || res == "random"));
                cmd += " -from " + res;

                // To
                do {
                    debugln("Receiver MAC address\n"
                            "  for example '00:20:91:aa:bb:5cc\n"
                            "  broadcast: visible to all devices");
                    CLI_READ_RES();
                } while (!(res.length() == 17 || res == "broadcast"));
                cmd += " -to " + res;

                // Encryption
                do {
                    debugln("Network encryption\n"
                            "  open: Open network (no password)\n"
                            "  wpa2: WPA2 protected network");
                    CLI_READ_RES();
                } while (!(res == "open" || res == "wpa2"));
                cmd += " -enc " + res;

                // Channel
                do {
                    debugln("Send on channel\n"
                            "  1-14: WiFi channel to send packets on");
                    CLI_READ_RES();
                } while (!(res.toInt() >= 1 && res.toInt() <= 14));
                cmd += " -ch " + res;

                // Time
                do {
                    debugln("Stop attack\n"
                            "   0: Never\n"
                            "  >0: After x seconds");
                    CLI_READ_RES();
                } while (!(res.toInt() >= 0));
                cmd += " -t " + res;
            } else if (res == "deauth") {}

            // Result
            for (int i = 0; i<cmd.length()+2; ++i) debug('#');
            debugln();
            debugln("Result:");

            cli::parse(cmd.c_str());
        });
        cmd_start.setDescription("  Start a guided tour through the functions of this device");

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            unsigned long time = 0;
            uint16_t channels  = 0;

            bool retain;
            bool ap = false;
            bool st = false;

            { // Scan time
                long seconds = cmd.getArg("t").getValue().toInt();
                if (seconds > 0) time = seconds*1000;
            }

            { // Channels
                String ch_str = cmd.getArg("ch").getValue();
                StringList ch_list(ch_str, ",");

                while (ch_list.available()) {
                    int ch = ch_list.iterate().toInt();

                    if ((ch >= 1) && (ch <= 14)) {
                        channels |= 1 << (ch-1);
                    }
                }
            }

            { // Retain results
                retain = cmd.getArg("r").isSet();
            }

            { // Mode
                String mode = cmd.getArg("m").getValue();
                if (mode == "ap") {
                    ap = true;
                } else if (mode == "st") {
                    st = true;
                } else if (mode == "ap+st") {
                    ap = true;
                    st = true;
                }
            }

            scan::search(ap, st, time, channels, retain);
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

        Command cmd_beacon = cli.addCommand("beacon", [](cmd* c) {
            Command cmd(c);

            StringList ssid_list;

            uint8_t from[6];
            uint8_t to[6];

            int enc = ENCRYPTION_OPEN;

            uint8_t ch;

            unsigned long timeout = 0;

            { // SSIDs
                String ssids = cmd.getArg("ssid").getValue();
                ssid_list.parse(ssids, ",");
            }

            { // MAC from
                String from_str = cmd.getArg("from").getValue();

                if (from_str.length() != 17) {
                    vendor::randomize(from);
                } else {
                    mac::fromStr(from_str.c_str(), from);
                }
            }

            { // MAC to
                String to_str = cmd.getArg("to").getValue();

                if (to_str.length() != 17) {
                    memcpy(to, mac::BROADCAST, 6);
                } else {
                    mac::fromStr(to_str.c_str(), to);
                }
            }

            { // Encryption
                String enc_str = cmd.getArg("enc").getValue();
                if (enc_str == "wpa2") enc = ENCRYPTION_WPA2;
            }

            { // Channel
                ch = cmd.getArg("ch").getValue().toInt();
            }

            { // Time
                long seconds = cmd.getArg("t").getValue().toInt();
                if (seconds > 0) timeout = seconds*1000;
            }

            attack::beacon(ssid_list, from, to, enc, ch, timeout);
        });
        cmd_beacon.addArg("s/sid/s");
        cmd_beacon.addArg("from,mac/from", "random");
        cmd_beacon.addArg("to,macto", "broadcast");
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

        Command cmd_deauth = cli.addCommand("deauth", [](cmd* c) {
            Command cmd(c);

            attack::TargetList targets;

            unsigned long timeout = 0;

            unsigned long max_pkts;
            unsigned long pkt_rate;

            bool deauth   = false;
            bool disassoc = false;

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

            { // Time
                long seconds = cmd.getArg("t").getValue().toInt();
                if (seconds > 0) timeout = seconds*1000;
            }

            { // Number
                max_pkts = cmd.getArg("n").getValue().toInt();
            }

            { // Rate
                pkt_rate = cmd.getArg("r").getValue().toInt();
            }

            { // Mode
                String mode = cmd.getArg("m").getValue();

                if (mode == "deauth+disassoc") {
                    deauth   = true;
                    disassoc = true;
                } else if (mode == "deauth") {
                    deauth = true;
                } else if (mode == "disassoc") {
                    disassoc = true;
                }
            }

            attack::deauth(targets, deauth, disassoc, pkt_rate, timeout, max_pkts);
        });
        cmd_deauth.addArg("ap", "");
        cmd_deauth.addArg("st/ation", "");
        cmd_deauth.addArg("mac", "");
        cmd_deauth.addArg("t/ime/out", "300");
        cmd_deauth.addArg("n/um/ber", "0");
        cmd_deauth.addArg("r/ate", "20");
        cmd_deauth.addArg("m/ode", "deauth+disassoc");
        cmd_deauth.setDescription(
            "  Deauthenticate (disconnect) selected WiFi connections\n"
            "  -ap:  access point IDs to attack\n"
            "  -st:  station IDs to attack\n"
            "  -mac: manual target selection [MacFrom-MacTo-Channel] for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'\n"
            "  -t:   attack timeout in seconds (default=300)\n"
            "  -n:   packet limit [>1] (default=0)\n"
            "  -r:   packets per second (default=20)\n"
            "  -m:   packet types [deauth,disassoc,deauth+disassoc] (default=deauth+disassoc)"
            );

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

        if (input.charAt(input.length()-1) == '\r') {
            input.remove(input.length()-1);
        }

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