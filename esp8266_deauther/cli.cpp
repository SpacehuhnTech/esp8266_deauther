/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "cli.h"

#include <SimpleCLI.h> // SimpleCLI library

#include "debug.h"
#include "scan.h"
#include "strh.h"
#include "StringList.h"
#include "mac.h"
#include "vendor.h"
#include "attack.h"
#include "alias.h"
#include "config.h"
#include "result_filter.h"

// ram usage
extern "C" {
  #include "user_interface.h"
}

#define CLI_READ_RES()\
    res = String();\
    while (!read(res));\
    if (res == String(F("exit")) || res == String(F("stop"))) {\
        debuglnF("Ok byeee");\
        return;\
    }

#define CLI_READ_RES_DEFAULT(_DEFAULT)\
    res = String();\
    while (!read(res, _DEFAULT));\
    if (res == String(F("exit")) || res == String(F("stop"))) {\
        debuglnF("Ok byeee");\
        return;\
    }

void rssi_meter_cb(int8_t rssi) {
    debugln(rssi);
}

namespace cli {
    // ===== PRIVATE ===== //
    SimpleCLI cli;                    // !< Instance of SimpleCLI library

#ifdef ENABLE_HISTORY
    StringList history(HISTORY_SIZE); // !< Command history
#endif // ifdef ENABLE_HISTORY

    void parse_mac(const String& str, uint8_t* mac) {
        // if in alias list, copy
        if (!alias::resolve(str, mac)) {
            if (str == "random") {
                vendor::randomize(mac);
            } else if (str == "broadcast") {
                memcpy(mac, mac::BROADCAST, 6);
            } else if (str.length() == 17) {
                mac::fromStr(str.c_str(), mac);
            }
        }
    }

    unsigned long parse_time(const String& str, unsigned long defaultMultiplicator) {
        unsigned long value = str.toInt();

        if (value > 0) {
            if (str.endsWith("ms")) value *= 1;
            else if (str.endsWith("s") || str.endsWith("sec")) value *= 1000;
            else if (str.endsWith("m") || str.endsWith("min")) value *= 60*1000;
            else if (str.endsWith("h")) value *= 60*60*1000;
            else value *= defaultMultiplicator;
        }

        return value;
    }

    SortedStringList parse_int_list(const String& str) {
        StringList values(str, ",");

        SortedStringList list;

        values.begin();

        while (values.available()) {
            String h = values.iterate();

            int hyphen = h.indexOf("-", 1);
            if ((hyphen >= 0) && (hyphen < h.length()-1)) {
                int start = h.substring(0, hyphen).toInt();
                int end   = h.substring(hyphen+1).toInt();

                for (int i = start; i<=end; ++i) {
                    list.push(String(i));
                }
            } else {
                list.push(String(h.toInt()));
            }
        }

        return list;
    }

    uint16_t parse_channels(const String& ch_str) {
        if (ch_str == "all") return 0x3FFF;

        SortedStringList ch_list = parse_int_list(ch_str);
        ch_list.begin();

        uint16_t channels = 0;

        while (ch_list.available()) {
            int ch = ch_list.iterate().toInt();

            if ((ch >= 1) && (ch <= 14)) {
                channels |= 1 << (ch-1);
            }
        }

        return channels;
    }

    // ===== PUBLIC ===== //
    void begin() {
        debug_init();

        debuglnF("\r\nSTARTED \\o/\r\n");

        debuglnF(VERSION);

        debuglnF("\r\n"
                 "[=================== DISCLAIMER ===================]\r\n"
                 "  This is a tool.\r\n"
                 "  It's neither good nor bad.\r\n"
                 "  Use it to study and test.\r\n"
                 "  Never use it to create harm or damage!\r\n"
                 "\r\n"
                 "  The continuation of this project counts on you!\r\n"
                 "[==================================================]\r\n");

        debuglnF("Type \"help\" to see all commands.\r\n"
                 "Type \"start\" to go through the functionalities step by step.");

        cli.setOnError([](cmd_error* e) {
            CommandError cmdError(e); // Create wrapper object

            debugF("ERROR: ");
            debug(cmdError.toString());

            if (cmdError.hasCommand()) {
                debugln();
                debugF("Did you mean:");
                debug(cmdError.getCommand().toString());
            }

            debugln();
        });

        Command cmd_help = cli.addCommand("help", [](cmd* c) {
            Command cmd(c);

            Argument cmdName = cmd.getArg("command");
            bool description = !cmd.getArg("short").isSet();

            if (cmdName.isSet()) {
                String cmdNameStr = cmdName.getValue();
                Command tmp       = cli.getCommand(cmdNameStr);
                if (tmp) {
                    debugln(tmp.toString(description));
                } else {
                    debug(cmdNameStr);
                    debuglnF(" not found :(");
                }
            } else {
                debug(cli.toString(description));
            }
        });
        cmd_help.addPosArg("cmd,command", "");
        cmd_help.addFlagArg("s/hort");
        cmd_help.setDescription("  Print the list of commands that you see right now");

        Command cmd_start = cli.addCommand("start", [](cmd* c) {
            String res;
            String cmd;

            debuglnF("Good morning friend!");
            debugln();

            { // Command
                do {
                    debuglnF("What can I do for you today?\r\n"
                             "  scan:    Search for WiFi networks and clients\r\n"
                             "  ---------\r\n"
                             "  beacon:  Send WiFi network advertisement beacons (spam network scanners)\r\n"
                             "  deauth:  Disrupt WiFi connections\r\n"
                             "  probe:   Send WiFi network requests (spam client scanners)\r\n"
                             "  ---------\r\n"
                             "  alias:   Give MAC addresses an alias\r\n"
                             "  results: Display and filter scan results\r\n"
                             "Remember that you can always escape by typing 'exit'");
                    CLI_READ_RES();
                } while (!(res == "scan" || res == "beacon" || res == "deauth" || res == "probe" || res == "alias" || res == "results"));
                cmd += res;
            }

            if (res == "scan") {
                { // Scan mode
                    do {
                        debuglnF("What do you wish to scan for?\r\n"
                                 "  ap:    Access points (WiFi networks)\r\n"
                                 "  st:    Stations (WiFi client devices)\r\n"
                                 "  ap+st: Access points and stations\r\n"
                                 " [default=ap+st]");
                        CLI_READ_RES_DEFAULT("ap+st");
                    } while (!(res == "ap" || res == "st" || res == "ap+st"));
                    if (res != "ap+st") cmd += " -m " + res;
                }

                // Scan time and channel(s)
                if (res != "ap") {
                    { // Scan time
                        do {
                            debuglnF("Scan for how long?\r\n"
                                     "  >1: Station scan time\r\n"
                                     " [default=20s]");
                            CLI_READ_RES_DEFAULT("20");
                        } while (!(res.toInt() > 0));
                        if (res != "20") cmd += " -t " + res;
                    }

                    { // Scan on channel(s)
                        debuglnF("Scan on wich channel(s)?\r\n"
                                 "  1-14: WiFi channel(s) to search on (for example: 1,6,11)\r\n"
                                 " [default=all]");
                        CLI_READ_RES_DEFAULT("all");
                        if ((res != "all") && (res != "1-14")) cmd += " -ch " + res;
                    }

                    { // Channel scan time
                        do {
                            debuglnF("Stay on each channel for how long?\r\n"
                                     "  >1: Channel time in milliseconds\r\n"
                                     " [default=284]");
                            CLI_READ_RES_DEFAULT("284");
                        } while (!(res.toInt() > 0) && res != "284");
                        if (res != "284") cmd += " -ct " + res;
                    }

                    { // Silent
                        do {
                            debuglnF("Enable silent mode (mute output)?\r\n"
                                     "  y: Yes\r\n"
                                     "  n: No\r\n"
                                     " [default=n]");
                            CLI_READ_RES_DEFAULT("n");
                        } while (!(res == String('y') || res == String('n')));
                        if (res == String('y')) cmd += " -s";
                    }
                }

                { // Retain scan results
                    do {
                        debuglnF("Keep previous scan results?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES_DEFAULT("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -r";
                }
            } else if (res == "beacon") {
                { // SSIDs
                    debuglnF("Which network names do you wish to advertise?\r\n"
                             "  for example: \"network A\",\"network B\"");
                    CLI_READ_RES();
                    cmd += " -ssid " + strh::escape(res);
                }

                { // BSSID
                    do {
                        debuglnF("What is the transmitter or sender address? (BSSID)\r\n"
                                 "  MAC address: for example '00:20:91:aa:bb:5c\r\n"
                                 "  random:      generate random MAC address\r\n"
                                 " [default=random]");
                        CLI_READ_RES_DEFAULT("random");
                    } while (!(res.length() == 17 || res == "random"));
                    if (res != "random") cmd += " -from " + res;
                }

                { // Receiver
                    do {
                        debuglnF("What is the receiver address?\r\n"
                                 "  MAC address: for example 00:20:91:aa:bb:5cc\r\n"
                                 "  broadcast:   send to everyone\r\n"
                                 " [default=broadcast]");
                        CLI_READ_RES_DEFAULT("broadcast");
                    } while (!(res.length() == 17 || res == "broadcast"));
                    if (res != "broadcast") cmd += " -to " + res;
                }

                { // Encryption
                    do {
                        debuglnF("What encryption should it use?\r\n"
                                 "  open: no encryption, an open network without a password\r\n"
                                 "  wpa2: WPA2 protected network\r\n"
                                 " [default=open]");
                        CLI_READ_RES_DEFAULT("open");
                    } while (!(res == "open" || res == "wpa2"));
                    if (res != "open") cmd += " -enc " + res;
                }

                { // Channel
                    do {
                        debuglnF("Which channel should be used?\r\n"
                                 "  1-14: WiFi channel to send packets on\r\n"
                                 " [default=1]");
                        CLI_READ_RES_DEFAULT("1");
                    } while (!(res.toInt() >= 1 && res.toInt() <= 14));
                    if (res != "1") cmd += " -ch " + res;
                }

                { // Time
                    do {
                        debuglnF("How long should the attack last?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Stop after x seconds\r\n"
                                 " [default=300]");
                        CLI_READ_RES_DEFAULT("300");
                    } while (!(res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                }

                { // Scan
                    do {
                        debuglnF("Scan for authentication requests?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES_DEFAULT("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -scan";
                }
            } else if (res == "deauth") {
                { // Target
                    do {
                        debuglnF("What do you want to deauthenticate?\r\n"
                                 "  ap:  a network\r\n"
                                 "  st:  a client device\r\n"
                                 "  mac: enter MAC addresses manually");
                        CLI_READ_RES();
                    } while (!(res == "ap" || res == "st" || res == "mac"));
                    cmd += " -"+res;

                    if (res == "ap") {
                        if (scan::getAccessPoints().size() == 0) {
                            debuglnF("ERROR: No access points in scan results.\r\n"
                                     "Type 'scan -m ap' to search for access points");
                            return;
                        }
                        scan::printAPs();

                        debuglnF("Select access point(s) to attack\r\n"
                                 "  >=0: ID(s) to select for the attack");
                        CLI_READ_RES();

                        cmd += " "+res;
                    } else if (res == "st") {
                        if (scan::getStations().size() == 0) {
                            debuglnF("ERROR: No stations in scan results.\r\n"
                                     "Type 'scan -m st' to search for stations");
                            return;
                        }
                        scan::printSTs();

                        debuglnF("Select station(s) to attack\r\n"
                                 "  >=0: ID(s) to select for the attack");
                        CLI_READ_RES();

                        cmd += " "+res;
                    } else if (res == "mac") {
                        debuglnF("Target(s) to attack\r\n"
                                 "  Sender-Receiver-Channel for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'");
                        CLI_READ_RES();

                        cmd += " "+res;
                    }
                }

                { // Noob filer
                    do {
                        debuglnF("Do you own or have permission to attack the selected devices?\r\n"
                                 "  yes\r\n"
                                 "  no");
                        CLI_READ_RES();
                    } while (!(res == "yes" || res == "no"));
                    if (res == "no") {
                        debuglnF("Then you should not attack them.\r\n"
                                 "It could get you in serious trouble.\r\n"
                                 "Please use this tool respectfully!");
                        return;
                    }
                }

                { // Time
                    do {
                        debuglnF("How long should the attack last?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Stop after x seconds\r\n"
                                 " [default=300]");
                        CLI_READ_RES_DEFAULT("300");
                    } while (!(res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                }

                { // Number of packets
                    do {
                        debuglnF("How many packets shall be sent?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Send x packets\r\n"
                                 " [default=0]");
                        CLI_READ_RES_DEFAULT("0");
                    } while (!(res.toInt() >= 0));
                    if (res != "0") cmd += " -n " + res;
                }

                { // Packet rate
                    do {
                        debuglnF("At which speed/rate?\r\n"
                                 "  >0 : Packets per second\r\n"
                                 " [default=20]");
                        CLI_READ_RES_DEFAULT("20");
                    } while (!(res.toInt() > 0));
                    if (res != "20") cmd += " -r " + res;
                }

                { // Mode
                    do {
                        debuglnF("What kind of packets shall be sent?\r\n"
                                 "  deauth:          Deauthentication\r\n"
                                 "  disassoc:        Disassociation\r\n"
                                 "  deauth+disassoc: Both\r\n"
                                 " [default=deauth+disassoc]");
                        CLI_READ_RES_DEFAULT("deauth+disassoc");
                    } while (!(res == "deauth" || res == "disassoc" || res == "deauth+disassoc"));
                    if (res != "deauth+disassoc") cmd += " -m " + res;
                }

                { // Silent
                    do {
                        debuglnF("Enable silent mode (mute output)?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES_DEFAULT("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -s";
                }
            } else if (res == "probe") {
                { // SSIDs
                    debuglnF("Which network names do you wish to request for?\r\n"
                             "  for example: \"network A\",\"network B\"");
                    CLI_READ_RES();
                    cmd += " -ssid " + strh::escape(res);
                }

                { // Receiver
                    do {
                        debuglnF("What is the receiver address?\r\n"
                                 "  MAC address: for example 00:20:91:aa:bb:5c\r\n"
                                 "  broadcast:   send to everyone\r\n"
                                 " [default=broadcast]");
                        CLI_READ_RES_DEFAULT("broadcast");
                    } while (!(res.length() == 17 || res == "broadcast"));
                    if (res != "broadcast") cmd += " -to " + res;
                }

                { // Channel
                    do {
                        debuglnF("Which channel should be used?\r\n"
                                 "  1-14: WiFi channel to send packets on\r\n"
                                 " [default=1]");
                        CLI_READ_RES_DEFAULT("1");
                    } while (!(res.toInt() >= 1 && res.toInt() <= 14));
                    if (res != "1") cmd += " -ch " + res;
                }

                { // Time
                    do {
                        debuglnF("How long should the attack last?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Stop after x seconds\r\n"
                                 " [default=300]");
                        CLI_READ_RES_DEFAULT("300");
                    } while (!(res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                }

                { // Silent
                    do {
                        debuglnF("Enable silent mode (mute output)?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES_DEFAULT("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -s";
                }
            } else if (res == "alias") {
                { // Mode
                    do {
                        debuglnF("Do you want to set a new alias, remove one or see the entire list?\r\n"
                                 "  list:   Print list of existing MAC address aliases\r\n"
                                 "  add:    Add new MAC address alias to list\r\n"
                                 "  remove: Remove an exisiting MAC address alias\r\n"
                                 " [default=list]");
                        CLI_READ_RES_DEFAULT("list");
                    } while (!(res == "list" || res == "add" || res == "remove"));
                    if (res != "list") cmd += " -mode " + res;
                }

                if (res != "list") {
                    { // Name
                        debuglnF("Alias (name):");
                        CLI_READ_RES();

                        if (res.length() > 0) cmd += " -name " + strh::escape(res);
                    }

                    { // MAC
                        debuglnF("MAC address:");
                        CLI_READ_RES();

                        if (res.length() > 0) cmd += " -mac " + res;
                    }
                }
            } else if (res == "results") {
                { // Type
                    do {
                        debuglnF("Filter type of results:\r\n"
                                 "  ap:    Access points (WiFi networks)\r\n"
                                 "  st:    Stations (WiFi client devices)\r\n"
                                 "  ap+st: Access points and stations\r\n"
                                 " [default=ap+st]");
                        CLI_READ_RES_DEFAULT("ap+st");
                    } while (!(res == "ap" || res == "st" || res == "ap+st"));
                    if (res != "ap+st") cmd += " -t " + res;

                    if ((res == "ap") && (scan::getAccessPoints().size() == 0)) {
                        debuglnF("ERROR: No access points in scan results.\r\n"
                                 "Type 'scan -m ap' to search for access points");
                        return;
                    } else if ((res == "st") && (scan::getStations().size() == 0)) {
                        debuglnF("ERROR: No stations in scan results.\r\n"
                                 "Type 'scan -m st' to search for stations");
                        return;
                    } else if ((res == "ap+st") && (scan::getAccessPoints().size() == 0) && (scan::getStations().size() == 0)) {
                        debuglnF("ERROR: No access points or stations in scan results.\r\n"
                                 "Type 'scan' to search for access points");
                        return;
                    }
                }

                { // Channel(s)
                    debuglnF("Filter channel(s):\r\n"
                             "  1-14: WiFi channel(s) to search on (for example: 1,6,11)\r\n"
                             " [default=all]");
                    CLI_READ_RES_DEFAULT("all");
                    if ((res != "all") && (res != "1-14")) cmd += " -ch " + res;
                }

                { // SSIDs
                    debuglnF("Filter for SSIDs (network names):\r\n"
                             "  for example: \"network A\",\"network B\"\r\n"
                             " [default=*No filter*]");
                    CLI_READ_RES_DEFAULT("");
                    if (res.length() > 0) cmd += " -ssid " + strh::escape(res);
                }

                { // BSSID
                    debuglnF("Filter for BSSID (MAC address of access point):\r\n"
                             "  for example 00:20:91:aa:bb:5cc\r\n"
                             " [default=*No filter*]");
                    CLI_READ_RES_DEFAULT("");
                    if (res.length() > 0) cmd += " -bssid " + res;
                }

                { // Vendor
                    debuglnF("Filter for vendor name:\r\n"
                             "  for example \"Apple\",\"Intel\"\r\n"
                             " [default=*No filter*]");
                    CLI_READ_RES_DEFAULT("");
                    if (res.length() > 0) cmd += " -vendor " + strh::escape(res);
                }
            }


            // Result
            for (int i = 0; i<cmd.length()+4 || i<21; ++i) debug('#');
            debugln();
            debuglnF("Exiting start command");

            for (int i = 0; i<cmd.length()+4 || i<21; ++i) debug('#');
            debugln();

            debugln();
            debugF("# ");
            debugln(cmd);

            cli::parse(cmd.c_str());
        });
        cmd_start.setDescription("  Start a guided tour through the functions of this device");

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            unsigned long timeout;
            unsigned long ch_time;
            uint16_t channels;
            bool silent;

            bool retain;
            bool ap = false;
            bool st = false;

            { // Station scan time
                String time_str = cmd.getArg("t").getValue();
                timeout         = parse_time(time_str, 1000);
            }

            { // Channels
                String ch_str = cmd.getArg("ch").getValue();
                channels      = parse_channels(ch_str);
            }

            { // Channel scan time
                String time_str = cmd.getArg("ct").getValue();
                ch_time         = parse_time(time_str, 1);
            }

            { // Silent
                silent = cmd.getArg("s").isSet();
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

            ap_scan_settings_t scan_settings;

            scan_settings.channels = channels;
            scan_settings.retain   = retain;
            scan_settings.st       = st;

            scan_settings.st_settings.channels = channels;
            scan_settings.st_settings.ch_time  = ch_time;
            scan_settings.st_settings.timeout  = timeout;
            scan_settings.st_settings.retain   = retain;

            if (ap) scan::startAP(scan_settings);
            else if (st) scan::startST(scan_settings.st_settings);
        });
        cmd_scan.addPosArg("m/ode", "ap+st");
        cmd_scan.addArg("t/ime", "20s");
        cmd_scan.addArg("ch/annel", "all");
        cmd_scan.addArg("ct/ime", "284");
        cmd_scan.addFlagArg("s/ilent");
        cmd_scan.addFlagArg("r/etain");
        cmd_scan.setDescription(
            "  Scan for WiFi devices\r\n"
            "  -m:  scan mode [ap,st,ap+st] (default=ap+st)\r\n"
            "  -t:  station scan time (default=20s)\r\n"
            "  -ch: 2.4 GHz channels for station scan [1-14] (default=all)\r\n"
            "  -ct: channel scan time in milliseconds (default=auto)\r\n"
            "  -s:  silent mode (mute output)\r\n"
            "  -r:  keep previous scan results"
            );

        /*
                Command cmd_rssi = cli.addCommand("rssi", [](cmd* c) {
                    Command cmd(c);
                    String mac_str = cmd.getArg("mac").getValue();

                    MACList macs(mac_str, ",");

                    scan::startRSSI(&rssi_meter_cb, macs);
                });
                cmd_rssi.addPosArg("mac", "");
                cmd_rssi.setDescription(
                    "  RSSI meter\r\n"
                    "  -mac: MAC addresses");
         */
        Command cmd_results = cli.addCommand("results", [](cmd* c) {
            Command cmd(c);
            String mode = cmd.getArg("t").getValue();

            String ch_str     = cmd.getArg("ch").getValue();
            String ssid_str   = cmd.getArg("ssid").getValue();
            String mac_str    = cmd.getArg("bssid").getValue();
            String vendor_str = cmd.getArg("vendor").getValue();

            uint16_t channels = parse_channels(ch_str);
            StringList ssids(ssid_str, ",");
            MACList macs(mac_str, ",");
            StringList vendors(vendor_str, ",");

            result_filter_t filter;
            filter.channels = channels;
            filter.ssids    = &ssids;
            filter.bssid    = &macs;
            filter.vendors  = &vendors;

            if (mode == "ap") {
                scan::printAPs(&filter);
            } else if (mode == "st") {
                scan::printSTs(&filter);
            } else if (mode == "ap+st") {
                scan::printAPs(&filter);
                scan::printSTs(&filter);
            }
        });
        cmd_results.addPosArg("t/ype", "ap+st");
        cmd_results.addArg("ch/annel/s", "all");
        cmd_results.addArg("ssid/s", "");
        cmd_results.addArg("bssid/s", "");
        cmd_results.addArg("vendor/s", "");
        cmd_results.setDescription(
            "  Print list of scan results [access points (networks) and stations (clients)]\r\n"
            "  -t:      type of results [ap,st,ap+st] (default=ap+st)\r\n"
            "  -ch:     filter by channel(s)\r\n"
            "  -ssid:   filter by SSID(s)\r\n"
            "  -bssid:  filter by BSSID(s)\r\n"
            "  -vendor: filter by vendor name(s)");

        Command cmd_beacon = cli.addCommand("beacon", [](cmd* c) {
            Command cmd(c);

            SortedStringList ssid_list;
            uint8_t bssid[6];
            uint8_t receiver[6];
            int enc = ENCRYPTION_OPEN;
            uint16_t channels;
            uint16_t pkt_rate;
            unsigned long timeout;
            bool silent;
            bool scan;

            { // SSIDs
                String ssids = cmd.getArg("ssid").getValue();
                ssid_list.parse(ssids, ",");
            }

            { // BSSID
                String bssid_str = cmd.getArg("from").getValue();
                parse_mac(bssid_str, bssid);
            }

            { // Receiver
                String receiver_str = cmd.getArg("to").getValue();
                parse_mac(receiver_str, receiver);
            }

            { // Encryption
                String enc_str = cmd.getArg("enc").getValue();
                if (enc_str == "wpa2") enc = ENCRYPTION_WPA2;
            }

            { // Channels
                String ch_str = cmd.getArg("ch").getValue();
                channels      = parse_channels(ch_str);
            }

            { // Packet rate
                String pkt_rate_str = cmd.getArg("r").getValue();
                pkt_rate            = pkt_rate_str.toInt();

                if (pkt_rate == 0) pkt_rate = 1;
                if (pkt_rate > 1000) pkt_rate = 1000;
            }

            { // Time
                String time_str = cmd.getArg("t").getValue();
                timeout         = parse_time(time_str, 1000);
            }

            { // Scan
                scan = cmd.getArg("mon").isSet();
            }

            attack::startBeacon(ssid_list, bssid, receiver, enc, channels, pkt_rate, timeout);
            if (scan) {
                auth_scan_settings_t auth_settings;

                auth_settings.channels  = channels;
                auth_settings.ch_time   = 1000/pkt_rate;
                auth_settings.timeout   = timeout;
                auth_settings.beacon    = true;
                auth_settings.receivers = nullptr;

                scan::startAuth(auth_settings);
            }
        });
        cmd_beacon.addPosArg("ssid/s");
        cmd_beacon.addArg("bssid,from", "random");
        cmd_beacon.addArg("receiver,to", "broadcast");
        cmd_beacon.addPosArg("enc/ryption", "open");
        cmd_beacon.addArg("ch/annel", "1");
        cmd_beacon.addArg("r/ate", "10");
        cmd_beacon.addArg("t/ime/out", "5min");
        cmd_beacon.addFlagArg("scan,auth,mon/itor");
        cmd_beacon.setDescription(
            "  Send WiFi network advertisement beacons\r\n"
            "  -ssid: network names (SSIDs) for example: \"test A\",\"test B\"\r\n"
            "  -from: BSSID or sender MAC address (default=random)\r\n"
            "  -to:   receiver MAC address (default=broadcast)\r\n"
            "  -enc:  encryption [open,wpa2] (default=open)\r\n"
            "  -ch:   channel (default=1)\r\n"
            "  -r:    packets per second per SSID (default=10)\r\n"
            "  -t:    attack timeout (default=5min)\r\n"
            "  -mon:  scan for authentications"
            );

        Command cmd_deauth = cli.addCommand("deauth", [](cmd* c) {
            Command cmd(c);

            TargetList targets;

            unsigned long timeout = 0;

            unsigned long max_pkts;
            unsigned long pkt_rate;

            bool deauth   = false;
            bool disassoc = false;

            bool silent;

            { // Read Access Point MACs
                String ap_str         = cmd.getArg("ap").getValue();
                SortedStringList list = parse_int_list(ap_str);
                list.begin();

                while (list.available()) {
                    int id          = list.iterate().toInt();
                    AccessPoint* ap = scan::getAccessPoints().get(id);
                    if (ap) {
                        targets.push(ap->getBSSID(), mac::BROADCAST, ap->getChannel());
                    }
                }
            }

            { // Read Station MACs
                String st_str         = cmd.getArg("st").getValue();
                SortedStringList list = parse_int_list(st_str);
                list.begin();

                while (list.available()) {
                    int id      = list.iterate().toInt();
                    Station* st = scan::getStations().get(id);
                    if (st) {
                        if (st->getAccessPoint()) {
                            targets.push(st->getAccessPoint()->getBSSID(), st->getMAC(), st->getAccessPoint()->getChannel());
                        } else {
                            debugF("WARNING: Station ");
                            debug(id);
                            debuglnF(" is not connected to an AP, therefor can't be deauthed.");
                        }
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

                    String sender_mac_str   = target_data.iterate();
                    String receiver_mac_str = target_data.iterate();
                    String ch_str           = target_data.iterate();

                    uint8_t sender_mac[6];
                    uint8_t receiver_mac[6];
                    uint8_t ch;

                    parse_mac(sender_mac_str, sender_mac);
                    parse_mac(receiver_mac_str, receiver_mac);
                    ch = ch_str.toInt();

                    targets.push(sender_mac, receiver_mac, ch);
                }
            }

            { // Time
                String time_str = cmd.getArg("t").getValue();
                timeout         = parse_time(time_str, 1000);
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

            { // Silent
                silent = cmd.getArg("s").isSet();
            }

            attack::startDeauth(targets, deauth, disassoc, pkt_rate, timeout, max_pkts, silent);
        });
        cmd_deauth.addArg("ap", "");
        cmd_deauth.addArg("st/ation", "");
        cmd_deauth.addArg("mac,manual", "");
        cmd_deauth.addArg("t/ime/out", "5min");
        cmd_deauth.addArg("n/um/ber", "0");
        cmd_deauth.addArg("r/ate", "20");
        cmd_deauth.addArg("m/ode", "deauth+disassoc");
        cmd_deauth.addFlagArg("s/ilent");
        cmd_deauth.setDescription(
            "  Deauthenticate (disconnect) selected WiFi connections\r\n"
            "  -ap:  access point IDs to attack\r\n"
            "  -st:  station IDs to attack\r\n"
            "  -mac: manual target selection [Sender-Receiver-Channel] for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'\r\n"
            "  -t:   attack timeout (default=5min)\r\n"
            "  -n:   packet limit [>1] (default=0)\r\n"
            "  -r:   packets per second (default=20)\r\n"
            "  -m:   packet types [deauth,disassoc,deauth+disassoc] (default=deauth+disassoc)\r\n"
            "  -s:   silent mode (mute output)"
            );

        Command cmd_probe = cli.addCommand("probe", [](cmd* c) {
            Command cmd(c);

            SortedStringList ssid_list;
            uint8_t receiver[6];
            uint8_t ch;
            unsigned long timeout = 0;
            bool silent;

            { // SSIDs
                String ssids = cmd.getArg("ssid").getValue();
                ssid_list.parse(ssids, ",");
            }

            { // Receiver
                String receiver_str = cmd.getArg("to").getValue();
                parse_mac(receiver_str, receiver);
            }

            { // Channel
                ch = cmd.getArg("ch").getValue().toInt();
            }

            { // Time
                String time_str = cmd.getArg("t").getValue();
                timeout         = parse_time(time_str, 1000);
            }

            { // Silent
                silent = cmd.getArg("s").isSet();
            }

            attack::startProbe(ssid_list, receiver, ch, timeout, silent);
        });
        cmd_probe.addPosArg("ssid/s");
        cmd_probe.addArg("receiver,to", "broadcast");
        cmd_probe.addArg("ch/annel", "1");
        cmd_probe.addArg("t/ime/out", "5min");
        cmd_probe.addFlagArg("s/ilent");
        cmd_probe.setDescription(
            "  Send probe requests for WiFi networks\r\n"
            "  -ssid: network names (SSIDs) for example: \"test A\",\"test B\"\r\n"
            "  -to:   receiver MAC address (default=broadcast)\r\n"
            "  -ch:   channel (default=1)\r\n"
            "  -t:    attack timeout (default=5min)\r\n"
            "  -s:    silent mode (mute output)"
            );

        Command cmd_alias = cli.addCommand("alias", [](cmd* c) {
            Command cmd(c);

            String mode = cmd.getArg("mode").getValue();

            if (mode == "list") {
                alias::print();
                return;
            }

            String name    = cmd.getArg("name").getValue();
            String mac_str = cmd.getArg("mac").getValue();

            if (mode == "add") {
                // No valid mac? Try switching arg values!
                if ((mac_str.length() != 17) || (mac_str.charAt(2) != ':')) {
                    String tmp = name;
                    name       = mac_str;
                    mac_str    = tmp;
                }
                uint8_t mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                parse_mac(mac_str, mac);

                if (alias::add(mac, name)) {
                    debugF("Alias \"");
                    debug(name);
                    debugF("\" for ");
                    debug(strh::mac(mac));
                    debuglnF(" saved");
                } else {
                    debuglnF("Something went wrong :(");
                    debuglnF("Invalid MAC address or already in list");
                }
                return;
            }

            if (mode == "remove") {
                uint8_t mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                parse_mac(name, mac);

                if (alias::remove(mac)) {
                    debugF("Removed alias ");
                    debugln(strh::mac(mac));
                } else if (alias::remove(name)) {
                    debugF("Removed alias ");
                    debugln(name);
                } else if (alias::remove(name.toInt())) {
                    debugF("Removed alias ");
                    debugln(mac_str);
                }
                return;
            }
        });
        cmd_alias.addPosArg("m/ode", "list");
        cmd_alias.addPosArg("name", "");
        cmd_alias.addPosArg("mac", "");
        cmd_alias.setDescription(
            "  Manage alias for MAC address\r\n"
            "  -mode: add,remove or list (default=list)\r\n"
            "  -name: alias name\r\n"
            "  -mac:  MAC address");

        Command cmd_clear = cli.addCommand("clear", [](cmd* c) {
            for (uint8_t i = 0; i<100; ++i) {
                debugln();
            }
        });
        cmd_clear.setDescription("  Clear serial output (by spamming line breaks :P)");

        Command cmd_ram = cli.addCommand("ram", [](cmd* c) {
            debugF("Size: ");
            debug(81920);
            debuglnF(" byte");

            debugF("Used: ");
            debug(81920 - system_get_free_heap_size());
            debugF(" byte (");
            debug(100 - (system_get_free_heap_size() / (81920 / 100)));
            debuglnF("%)");

            debugF("Free: ");
            debug(system_get_free_heap_size());
            debugF(" byte (");
            debug(system_get_free_heap_size() / (81920 / 100));
            debuglnF("%)");
        });
        cmd_ram.setDescription("  Print memory usage");

        Command cmd_stop = cli.addCommand("stop", [](cmd* c) {
            scan::stop();
            attack::stop();
        });
        cmd_stop.setDescription("  Stop all attacks");

#ifdef ENABLE_HISTORY
        Command cmd_history = cli.addCommand("history", [](cmd* c) {
            debuglnF("Command history:");

            history.begin();

            while (history.available()) {
                debugF("  ");
                debugln(history.iterate());
            }
        });
        cmd_history.setDescription("  Print previous 10 commands");
#endif // ifdef ENABLE_HISTORY

        Command cmd_reboot = cli.addCommand("restart,reboot", [](cmd* c) {
            ESP.restart();
        });
        cmd_reboot.setDescription("  Restarts/Reboots the ESP8266");

        Command cmd_chicken = cli.addCommand("chicken", [](cmd* c) {
            debugF("                                                                 ` - : /////////:-.\r\n"
                   "                                                            ./++so:`   ``    `.:/++/.\r\n"
                   "                                                        `/+o+.+o:.s:-++//s`        `:++-\r\n"
                   "                                                     `/+oo+//d-   oh/    s-            :o/`\r\n"
                   "                                                   .++.o+` `h-   .d.     h`://+`         .o+\r\n"
                   "                                                 .o+` +/   +o    y-     +d+.  .y           .s-\r\n"
                   "                                                +o`   h    d`   `/     .h-    `h             ++\r\n"
                   "                                              .s-     d    -           .`     +/              /o\r\n"
                   "                                             :o`      y.                     -y                /+\r\n"
                   "                                            /+        :o       `            -h`                 s:\r\n"
                   "                                           /o          y..://///////:`     /o/o.                `h\r\n"
                   "                                          -s           +o:``       `-++::/+-  `o/                o:\r\n"
                   "                                          y.          :o              `:::.`   `oo`              -s\r\n"
                   "                                         -s           h`            .++:---/+/+/:::++.           `h\r\n"
                   "                                         +/           h            :o`   `  `/s  `  .s-           d\r\n"
                   "                                         o:          .s            h`   /h-   o:/h-  -s          `h\r\n"
                   "                                         +/          +/            h`   ``    s- `   +/          -s\r\n"
                   "                                         .y         `h`            -s-      `+y-.`.:+/           +:\r\n"
                   "                                          o:        o:              `/+/:/+ss:.-:y/.`           `h`\r\n"
                   "   .:-`                                   `y-      ++                  `so::-://+y.             +/\r\n"
                   "  :o.-/+:  :+//:`                          `s:   `+/                   -h//:::---:/o-          -y\r\n"
                   "  :o   `:o:h. `-+/`                         -d+.:o-                    .y.``...-/y/++`        `y.\r\n"
                   "   +/    `:hs    -o-                        o/:/yo:-`                   +y++s//+/.           `s.\r\n"
                   "    /o`    `oo`   `/+`                   .-:y/-`+:+so+/:-`              s-  y:              -s.\r\n"
                   " ++//+y:     -+     .o:            ``-:/+:-.`.:+/:hs+`++:/o/:.`        `h   .y`            /o`\r\n"
                   "`h` `./ys-            :o-   .--:////:-.`        `-/o/::.`/sh-:os/:.`   .y oo`+/          -o:\r\n"
                   " :o-   `-o+.           `/+o/:-..`                   `.:+++o/``/:-oo++/:.so+://`       `:+/`\r\n"
                   "  `/+:`   ..             `++`                           `.-/+/:-/sy.`+o:+y/-.      .-/+-`\r\n"
                   "    `-+/-                  .-                                `.:/o+:-:.```-:oy/:://:-`\r\n"
                   "       .:+/.                                                      `.-:/+/::s/-..`\r\n"
                   "          .++.                                                          `.-h.\r\n"
                   "            .o/                                                            +/\r\n"
                   "              :o.                                                          :o\r\n"
                   "               .o:                                                         -s\r\n"
                   "                 /o`                                                       :+\r\n"
                   "                  -o-                                                      o:\r\n"
                   "                   `o/                                                     h`\r\n"
                   "                     :o.                                                  -s\r\n"
                   "                      .o:                                                 y.\r\n"
                   "                        /o.                                              /+\r\n"
                   "                         .+/`                                           -s\r\n"
                   "                           -+/.                                        .s`\r\n"
                   "                             ./+/.`                                   -s`\r\n"
                   "                                .:/+:.`                              /o`\r\n"
                   "                                    .:/o/.`                        .o:\r\n"
                   "                                       o/:/+/.`                  .++`\r\n"
                   "                                       -s   `:/+:`            `:+/`\r\n"
                   "                                        ++`     -+o-`      `-++-\r\n"
                   "                                         :s/::/+//::+/---/+/:`\r\n"
                   "                                          +/s:`      `-h-s-\r\n"
                   "                                          +/s-        `y y.\r\n"
                   "                                          +/y.        `y h`\r\n"
                   "                                          //s:`       `y d\r\n"
                   "                                          +/-:/++/-`  `y h-`\r\n"
                   "                                          y:hs-ysosss..y --/+++/-`\r\n"
                   "                                          ds:`s:o+`-:`o:oos./h++osoo`\r\n"
                   "                                          ::   o+++   h:y `o+.s:`.::\r\n"
                   "                                                -+-   -/`   :s.++\r\n"
                   "                                                             `/+-\r\n");
        });
        cmd_chicken.setDescription("  Needs more chicken!");
    }

    void parse(const char* input) {
        cli.parse(input);
    }

    bool available() {
        return debug_available();
    }

    bool read(String& dest, const char* _default) {
        bool printed = false;

        while (true) {
            // Wait until input is available
            if (!debug_available()) {
                yield();
                continue;
            }

            // Read next character
            int c = debug_read();

            // Printable ASCII characters
            if ((c >= ' ') && (c <= '~')) {
                // Print prefix before the first char
                if (!printed) {
                    debugF("\r\n# ");
                    printed = true;
                }

                // Print char and add it to the string
                debug(char(c));
                dest += char(c);
            }

            // If end of line, check next char and break the loop
            else if ((c == '\r') || (c == '\n')) {
                delay(1);
                c = debug_peek();
                if ((c == '\r') || (c == '\n')) debug_read();

                break;
            }

            // Backspace key
            else if ((c == 127) || (c == 8)) {
                int len = dest.length();
                if (len > 0) {
                    dest.remove(len-1);

                    debugF("\r\n# ");
                    debug(dest);
                    printed = true;
                }
            }
        }

        // No input? Use default value if available.
        if (!printed && _default) {
            dest += String(_default);

            debugF("\r\n# ");
            debug(_default);

            printed = true;
        }

        // Add linebreak and print empty line after input
        if (printed) {
            debugln();
            debugln();
            return true;
        }

        return false;
    }

    bool read_exit() {
        if (debug_available()) {
            String input;
            read(input);
            return input == "stop" || input == "exit";
        }
        return false;
    }

    void update() {
        if (debug_available()) {
            //  String input = debug_read();
            String input;
            read(input);

            if (input.length() > 0) {
                cli::parse(input.c_str());
#ifdef ENABLE_HISTORY
                history.forcePush(input);
#endif // ifdef ENABLE_HISTORY
            }
        }
    }
}