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
#include "scan_filter.h"

// ram usage
extern "C" {
  #include "user_interface.h"
}

#define CLI_READ_RES(_DEFAULT)\
    res = read_and_wait(_DEFAULT);\
    if (res == "exit" || res == "stop") {\
        debuglnF("Ok byeee");\
        return;\
    }

namespace cli {
    // ===== PRIVATE ===== //
    SimpleCLI cli;                    // !< Instance of SimpleCLI library

#ifdef ENABLE_HISTORY
    StringList history(HISTORY_SIZE); // !< Command history
#endif // ifdef ENABLE_HISTORY

    uint16_t getChannels(const String& ch_str) {
        if (ch_str == "all") return 0x3FFF;

        StringList ch_list(ch_str, ",");

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

        debuglnF("\r\nDISCLAIMER:\r\n"
                 "This is a tool.\r\n"
                 "It's neither good nor bad.\r\n"
                 "Use it to study and test.\r\n"
                 "Never use it to create harm or damage!\r\n"
                 "\r\n"
                 "The continuation of this project counts on you!\r\n");

        debuglnF("Type \"help\" to see all commands.\r\n"
                 "Type \"start\" to go through the functionalities step by step.\r\n");


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
            debugln(cli.toString());
        });
        cmd_help.setDescription("  Print the list of commands that you see right now");

        Command cmd_start = cli.addCommand("start", [](cmd* c) {
            String res;
            String cmd;

            debuglnF("Good morning friend!");
            debugln();

            { // Command
                do {
                    debuglnF("What can I do for you today?\r\n"
                             "  scan:   Search for WiFi networks and clients\r\n"
                             "  beacon: Send WiFi network advertisement beacons (spam network scanners)\r\n"
                             "  deauth: Disrupt WiFi connections\r\n"
                             "  probe:  Send WiFi network requests (spam client scanners)\r\n"
                             "  alias:  Give MAC addresses an alias\r\n"
                             "Remember that you can always escape by typing 'exit'"
                             );
                    CLI_READ_RES();
                } while (!(res == "scan" || res == "beacon" || res == "deauth" || res == "probe" || res == "alias"));
                cmd += res;
                debugln();
            }

            if (res == "scan") {
                { // Scan mode
                    do {
                        debuglnF("What do you wish to scan for?\r\n"
                                 "  ap:    Access points (WiFi networks)\r\n"
                                 "  st:    Stations (WiFi client devices)\r\n"
                                 "  ap+st: Access points and stations\r\n"
                                 " [default=ap+st]"
                                 );
                        CLI_READ_RES("ap+st");
                    } while (!(res == "ap" || res == "st" || res == "ap+st"));
                    if (res != "ap+st") cmd += " -m " + res;
                    debugln();
                }

                // Scan time and channel(s)
                if (res != "ap") {
                    { // Scan time
                        do {
                            debuglnF("Scan for how long?\r\n"
                                     "  >1: Station scan time in seconds\r\n"
                                     " [default=14]");
                            CLI_READ_RES("14");
                        } while (!(res.toInt() > 0));
                        if (res != "14") cmd += " -t " + res;
                        debugln();
                    }

                    { // Scan on channel(s)
                        debuglnF("Scan on wich channel(s)?\r\n"
                                 "  1-14: WiFi channel(s) to search on (for example: 1,6,11)\r\n"
                                 " [default=all]");
                        CLI_READ_RES("all");
                        if (res != "all") cmd += " -ch " + res;
                        debugln();
                    }

                    { // Channel scan time
                        do {
                            debuglnF("Stay on each channel for how long?\r\n"
                                     "  >1: Channel time in milliseconds\r\n"
                                     " [default=auto]");
                            CLI_READ_RES("auto");
                        } while (!(res.toInt() > 0) && res != "auto");
                        if (res != "auto") cmd += " -ct " + res;
                        debugln();
                    }

                    { // Silent
                        do {
                            debuglnF("Enable silent mode (mute output)?\r\n"
                                     "  y: Yes\r\n"
                                     "  n: No\r\n"
                                     " [default=n]");
                            CLI_READ_RES("n");
                        } while (!(res == String('y') || res == String('n')));
                        if (res == String('y')) cmd += " -s";
                        debugln();
                    }
                }

                { // Retain scan results
                    do {
                        debuglnF("Keep previous scan results?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -r";
                    debugln();
                }
            } else if (res == "beacon") {
                { // SSIDs
                    debuglnF("Which network names do you wish to advertise?\r\n"
                             "  for example: \"network A\",\"network B\"");
                    CLI_READ_RES();
                    cmd += " -ssid " + res;
                    debugln();
                }

                { // From
                    do {
                        debuglnF("Who is the transmitter/sender?\r\n"
                                 "  MAC address: for example '00:20:91:aa:bb:5c\r\n"
                                 "  random:      generate random MAC address\r\n"
                                 " [default=random]");
                        CLI_READ_RES("random");
                    } while (!(res.length() == 17 || res == "random"));
                    if (res != "random") cmd += " -from " + res;
                    debugln();
                }

                { // To
                    do {
                        debuglnF("Who is the receiver?\r\n"
                                 "  MAC address: for example 00:20:91:aa:bb:5cc\r\n"
                                 "  broadcast:   send to everyone\r\n"
                                 " [default=broadcast]");
                        CLI_READ_RES("broadcast");
                    } while (!(res.length() == 17 || res == "broadcast"));
                    if (res != "broadcast") cmd += " -to " + res;
                    debugln();
                }

                { // Encryption
                    do {
                        debuglnF("What encryption should it use?\r\n"
                                 "  open: no encryption, an open network without a password\r\n"
                                 "  wpa2: WPA2 protected network\r\n"
                                 " [default=open]");
                        CLI_READ_RES("open");
                    } while (!(res == "open" || res == "wpa2"));
                    cmd += " -enc " + res;
                    debugln();
                }

                { // Channel
                    do {
                        debuglnF("Which channel should be used?\r\n"
                                 "  1-14: WiFi channel to send packets on\r\n"
                                 " [default=1]");
                        CLI_READ_RES("1");
                    } while (!(res.toInt() >= 1 && res.toInt() <= 14));
                    if (res != "1") cmd += " -ch " + res;
                    debugln();
                }

                { // Time
                    do {
                        debuglnF("How long should the attack last?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Stop after x seconds\r\n"
                                 " [default=300]");
                        CLI_READ_RES("300");
                    } while (!(res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                    debugln();
                }

                { // Silent
                    do {
                        debuglnF("Enable silent mode (mute output)?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -s";
                    debugln();
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
                    debugln();

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
                                 "  MacFrom-MacTo-Channel for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'");
                        CLI_READ_RES();

                        cmd += " "+res;
                    }
                    debugln();
                }

                { // Noob filer
                    do {
                        debuglnF("Do you own or have permission to attack the selected devices?\r\n"
                                 "  yes\r\n"
                                 "  no");
                        CLI_READ_RES();
                    } while (!(res == "yes" || res == "no"));
                    debugln();
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
                        CLI_READ_RES("300");
                    } while (!(res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                    debugln();
                }

                { // Number of packets
                    do {
                        debuglnF("How many packets shall be sent?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Send x packets\r\n"
                                 " [default=0]");
                        CLI_READ_RES("0");
                    } while (!(res.toInt() >= 0));
                    if (res != "0") cmd += " -n " + res;
                    debugln();
                }

                { // Packet rate
                    do {
                        debuglnF("At which speed/rate?\r\n"
                                 "  >0 : Packets per second\r\n"
                                 " [default=20]");
                        CLI_READ_RES("20");
                    } while (!(res.toInt() > 0));
                    if (res != "20") cmd += " -r " + res;
                    debugln();
                }

                { // Mode
                    do {
                        debuglnF("What kind of packets shall be sent?\r\n"
                                 "deauth:          Deauthentication\r\n"
                                 "disassoc:        Disassociation\r\n"
                                 "deauth+disassoc: Both\r\n"
                                 " [default=deauth+disassoc]");
                        CLI_READ_RES("deauth+disassoc");
                    } while (!(res == "deauth" || res == "disassoc" || res == "deauth+disassoc"));
                    if (res != "deauth+disassoc") cmd += " -m " + res;
                    debugln();
                }

                { // Silent
                    do {
                        debuglnF("Enable silent mode (mute output)?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -s";
                    debugln();
                }
            } else if (res == "probe") {
                { // SSIDs
                    debuglnF("Which network names do you wish to request for?\r\n"
                             "  for example: \"network A\",\"network B\"");
                    CLI_READ_RES();
                    cmd += " -ssid " + res;
                    debugln();
                }

                { // To
                    do {
                        debuglnF("Who is the receiver?\r\n"
                                 "  MAC address: for example 00:20:91:aa:bb:5cc\r\n"
                                 "  broadcast:   send to everyone\r\n"
                                 " [default=broadcast]");
                        CLI_READ_RES("broadcast");
                    } while (!(res.length() == 17 || res == "broadcast"));
                    if (res != "broadcast") cmd += " -to " + res;
                    debugln();
                }

                { // Channel
                    do {
                        debuglnF("Which channel should be used?\r\n"
                                 "  1-14: WiFi channel to send packets on\r\n"
                                 " [default=1]");
                        CLI_READ_RES("1");
                    } while (!(res.toInt() >= 1 && res.toInt() <= 14));
                    if (res != "1") cmd += " -ch " + res;
                    debugln();
                }

                { // Time
                    do {
                        debuglnF("How long should the attack last?\r\n"
                                 "   0: Infinite\r\n"
                                 "  >0: Stop after x seconds\r\n"
                                 " [default=300]");
                        CLI_READ_RES("300");
                    } while (!(res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                    debugln();
                }

                { // Silent
                    do {
                        debuglnF("Enable silent mode (mute output)?\r\n"
                                 "  y: Yes\r\n"
                                 "  n: No\r\n"
                                 " [default=n]");
                        CLI_READ_RES("n");
                    } while (!(res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -s";
                    debugln();
                }
            } else if (res == "alias") {
                { // Mode
                    do {
                        debuglnF("Do you want to set a new alias, remove one or see the entire list?\r\n"
                                 "  list:   Print list of existing MAC address aliases\r\n"
                                 "  add:    Add new MAC address alias to list\r\n"
                                 "  remove: Remove an exisiting MAC address alias\r\n"
                                 " [default=list]"
                                 );
                        CLI_READ_RES("list");
                    } while (!(res == "list" || res == "add" || res == "remove"));
                    if (res != "list") cmd += " -mode " + res;
                    debugln();
                }

                if (res != "list") {
                    { // Name
                        debuglnF("Alias (name):");
                        CLI_READ_RES();

                        if (res.length() > 0) cmd += " -name \"" + res + "\"";
                        debugln();
                    }

                    { // MAC
                        debuglnF("MAC address:");
                        CLI_READ_RES();

                        if (res.length() > 0) cmd += " -mac " + res;
                        debugln();
                    }
                }
            }


            // Result
            for (int i = 0; i<cmd.length()+4 || i<21; ++i) debug('#');
            debugln();
            debuglnF("Exiting start command");

            for (int i = 0; i<cmd.length()+4 || i<21; ++i) debug('#');
            debugln();

            cli::parse(cmd.c_str());
        });
        cmd_start.setDescription("  Start a guided tour through the functions of this device");

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            unsigned long time    = 0;
            unsigned long ch_time = 0;
            uint16_t channels     = 0;
            bool silent;

            bool retain;
            bool ap = false;
            bool st = false;

            { // Station scan time
                long seconds = cmd.getArg("t").getValue().toInt();
                if (seconds > 0) time = seconds*1000;
            }

            { // Channels
                String ch_str = cmd.getArg("ch").getValue();
                channels      = getChannels(ch_str);
            }

            { // Channel scan time
                ch_time = cmd.getArg("ct").getValue().toInt();
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

            scan::start(ap, st, time, channels, ch_time, silent, retain);
        });
        cmd_scan.addPosArg("m/ode", "ap+st");
        cmd_scan.addArg("t/ime", "14");
        cmd_scan.addArg("ch/annel", "1,2,3,4,5,6,7,8,9,10,11,12,13,14");
        cmd_scan.addArg("ct/ime", "auto");
        cmd_scan.addFlagArg("s/ilent");
        cmd_scan.addFlagArg("r/etain");
        cmd_scan.setDescription(
            "  Scan for WiFi devices\r\n"
            "  -m:  scan mode [ap,st,ap+st] (default=ap+st)\r\n"
            "  -t:  station scan time in seconds [>1] (default=14)\r\n"
            "  -ch: 2.4 GHz channels for station scan [1-14] (default=all)\r\n"
            "  -ct: channel scan time in milliseconds [>1] (default=auto)\r\n"
            "  -s:  silent mode (mute output)\r\n"
            "  -r:  keep previous scan results"
            );

        Command cmd_results = cli.addCommand("results", [](cmd* c) {
            Command cmd(c);
            String mode = cmd.getArg("t").getValue();

            String ch_str     = cmd.getArg("ch").getValue();
            String ssid_str   = cmd.getArg("ssid").getValue();
            String mac_str    = cmd.getArg("bssid").getValue();
            String vendor_str = cmd.getArg("vendor").getValue();

            uint16_t channels = getChannels(ch_str);
            StringList ssids(ssid_str, ",");
            MACList macs(mac_str, ",");
            StringList vendors(vendor_str, ",");

            scan_filter_t filter;
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
        cmd_results.addArg("bssid", "");
        cmd_results.addArg("vendor/s", "");
        cmd_results.setDescription(
            "  Print list of scan results [access points (networks) and stations (clients)]\r\n"
            "  -t:      type of results [ap,st,ap+st] (default=ap+st)\r\n"
            "  -ch:     filter by channel(s)\r\n"
            "  -ssid:   filter by SSID(s)\r\n"
            "  -bssid:  filter by BSSID\r\n"
            "  -vendor: filter by vendor name(s)");

        Command cmd_beacon = cli.addCommand("beacon", [](cmd* c) {
            Command cmd(c);

            SortedStringList ssid_list;
            uint8_t from[6];
            uint8_t to[6];
            int enc = ENCRYPTION_OPEN;
            uint8_t ch;
            unsigned long timeout = 0;
            bool silent;

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

            { // Silent
                silent = cmd.getArg("s").isSet();
            }

            attack::startBeacon(ssid_list, from, to, enc, ch, timeout, silent);
            scan::startAuth(from, timeout, silent);
        });
        cmd_beacon.addPosArg("ssid/s");
        cmd_beacon.addArg("from,mac/from,bssid", "random");
        cmd_beacon.addArg("to,macto", "broadcast");
        cmd_beacon.addPosArg("enc/ryption", "open");
        cmd_beacon.addArg("ch/annel", "1");
        cmd_beacon.addArg("t/ime", "300");
        cmd_beacon.addFlagArg("s/ilent");
        cmd_beacon.setDescription(
            "  Send WiFi network advertisement beacons\r\n"
            "  -ssid: network names (SSIDs) for example: \"test A\",\"test B\"\r\n"
            "  -from: sender MAC address (default=random)\r\n"
            "  -to:   receiver MAC address (default=broadcast)\r\n"
            "  -enc:  encryption [open,wpa2] (default=open)\r\n"
            "  -ch:   channel (default=1)\r\n"
            "  -t:    attack timeout in seconds (default=300)\r\n"
            "  -s:    silent mode (mute output)"
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
                String ap_str = cmd.getArg("ap").getValue();
                StringList list(ap_str, ",");

                while (list.available()) {
                    AccessPoint* ap = scan::getAccessPoints().get(list.iterate().toInt());
                    if (ap) {
                        targets.push(ap->getBSSID(), mac::BROADCAST, ap->getChannel());
                    }
                }
            }

            { // Read Station MACs
                String st_str = cmd.getArg("st").getValue();
                StringList list(st_str, ",");

                while (list.available()) {
                    Station* st = scan::getStations().get(list.iterate().toInt());
                    if (st && st->getAccessPoint()) {
                        targets.push(st->getAccessPoint()->getBSSID(), st->getMAC(), st->getAccessPoint()->getChannel());
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

            { // Silent
                silent = cmd.getArg("s").isSet();
            }

            attack::startDeauth(targets, deauth, disassoc, pkt_rate, timeout, max_pkts, silent);
        });
        cmd_deauth.addArg("ap", "");
        cmd_deauth.addArg("st/ation", "");
        cmd_deauth.addArg("mac", "");
        cmd_deauth.addArg("t/ime/out", "300");
        cmd_deauth.addArg("n/um/ber", "0");
        cmd_deauth.addArg("r/ate", "20");
        cmd_deauth.addArg("m/ode", "deauth+disassoc");
        cmd_deauth.addFlagArg("s/ilent");
        cmd_deauth.setDescription(
            "  Deauthenticate (disconnect) selected WiFi connections\r\n"
            "  -ap:  access point IDs to attack\r\n"
            "  -st:  station IDs to attack\r\n"
            "  -mac: manual target selection [MacFrom-MacTo-Channel] for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'\r\n"
            "  -t:   attack timeout in seconds (default=300)\r\n"
            "  -n:   packet limit [>1] (default=0)\r\n"
            "  -r:   packets per second (default=20)\r\n"
            "  -m:   packet types [deauth,disassoc,deauth+disassoc] (default=deauth+disassoc)\r\n"
            "  -s:   silent mode (mute output)"
            );

        Command cmd_probe = cli.addCommand("probe", [](cmd* c) {
            Command cmd(c);

            SortedStringList ssid_list;
            uint8_t to[6];
            uint8_t ch;
            unsigned long timeout = 0;
            bool silent;

            { // SSIDs
                String ssids = cmd.getArg("ssid").getValue();
                ssid_list.parse(ssids, ",");
            }

            { // MAC to
                String to_str = cmd.getArg("to").getValue();

                if (to_str.length() != 17) {
                    memcpy(to, mac::BROADCAST, 6);
                } else {
                    mac::fromStr(to_str.c_str(), to);
                }
            }

            { // Channel
                ch = cmd.getArg("ch").getValue().toInt();
            }

            { // Time
                long seconds = cmd.getArg("t").getValue().toInt();
                if (seconds > 0) timeout = seconds*1000;
            }

            { // Silent
                silent = cmd.getArg("s").isSet();
            }

            attack::startProbe(ssid_list, to, ch, timeout, silent);
        });
        cmd_probe.addPosArg("ssid/s");
        cmd_probe.addArg("to,macto", "broadcast");
        cmd_probe.addArg("ch/annel", "1");
        cmd_probe.addArg("t/ime", "300");
        cmd_probe.addFlagArg("s/ilent");
        cmd_probe.setDescription(
            "  Send probe requests for WiFi networks\r\n"
            "  -ssid: network names (SSIDs) for example: \"test A\",\"test B\"\r\n"
            "  -to:   receiver MAC address (default=broadcast)\r\n"
            "  -ch:   channel (default=1)\r\n"
            "  -t:    attack timeout in seconds (default=300)\r\n"
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
                mac::fromStr(mac_str.c_str(), mac);

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
                mac::fromStr(name.c_str(), mac);

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
            attack::stop();
            scan::stop();
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
    }

    void parse(const char* input) {
        debugF("# ");
        debug(input);
        debugln();
        debugln();

        cli.parse(input);
    }

    bool available() {
        return debug_available();
    }

    String read(String _default) {
        String input = debug_read();

        if (input.charAt(input.length()-1) == '\r') {
            input.remove(input.length()-1);
        }

        if (input.length() == 0) {
            input = _default;
        }

        debugF("# ");
        debugln(input);

        return input;
    }

    String read_and_wait(String _default) {
        while (!debug_available()) delay(1);
        return read(_default);
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
#ifdef ENABLE_HISTORY
            history.forcePush(input);
#endif // ifdef ENABLE_HISTORY
        }
    }
}