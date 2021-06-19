/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "cli.h"

#include "src/SimpleCLI-1.1.1/SimpleCLI.h"

#if SIMPLECLI_VERSION_MAJOR == 1 && SIMPLECLI_VERSION_MINOR < 1
    #error "Please update SimpleCLI library"
#endif // if SIMPLECLI_VERSION_MAJOR == 1 && SIMPLECLI_VERSION_MINOR < 1

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
#include "ap.h"

// ram usage
extern "C" {
  #include "user_interface.h"
}

#define CLI_READ_RES(_TEXT)\
    debuglnF(_TEXT);\
    res = String();\
    while (!read(res));\
    if (res == String(F("exit")) || res == String(F("stop"))) {\
        debuglnF("Ok byeee");\
        return;\
    }

#define CLI_READ_RES_DEFAULT(_TEXT, _DEFAULT)\
    debuglnF(_TEXT);\
    debugF("[default=");\
    debug(_DEFAULT);\
    debuglnF("]");\
    res = String();\
    while (!read(res, _DEFAULT));\
    if ((res == String(F("exit"))) || (res == String(F("stop")))) {\
        debuglnF("Ok byeee");\
        return;\
    }

#define CLI_READ_UNTIL(_TEXT, _STOP)\
    do {\
        CLI_READ_RES(_TEXT);\
    }\
    while (!_STOP);

#define CLI_READ_DEFAULT_UNTIL(_TEXT, _DEFAULT, _STOP)\
    do {\
        CLI_READ_RES_DEFAULT(_TEXT, _DEFAULT);\
    }\
    while (!_STOP);

void rssi_meter_cb(int8_t rssi) {
    debugln(rssi);
}

namespace cli {
    // ===== PRIVATE ===== //
    SimpleCLI cli { 64, 64 };         // !< Instance of SimpleCLI library

    unsigned long timer = 0;          // !< Timestamp to wakeup, if sleep command was used

#ifdef ENABLE_HISTORY
    StringList history(HISTORY_SIZE); // !< Command history
#endif // ifdef ENABLE_HISTORY

    void parse_mac(const String& str, uint8_t* mac) {
        // if in alias list, copy
        if (!alias::resolve(str, mac)) {
            if (str == "random") {
                vendor::getRandomMac(mac);
            } else if (str == "broadcast") {
                memcpy(mac, mac::BROADCAST, 6);
            } else {
                mac::fromStr(str.c_str(), mac);
            }
        }
    }

    unsigned long parse_time(const String& str, unsigned long defaultMultiplicator) {
        StringList list(str, "+");

        list.begin();

        unsigned long value { 0 };

        while (list.available()) {
            String str { list.iterate() };
            long   str_value { str.toInt() };

            if (str_value > 0) {
                if (str.endsWith("ms")) str_value *= 1;
                else if (str.endsWith("s") || str.endsWith("sec")) str_value *= 1000;
                else if (str.endsWith("m") || str.endsWith("min")) str_value *= 60*1000;
                else if (str.endsWith("h")) str_value *= 60*60*1000;
                else str_value *= defaultMultiplicator;

                value += str_value;
            }
        }

        return value;
    }

    SortedStringList parse_int_list(const String& str) {
        StringList values { str };

        SortedStringList list;

        values.begin();

        while (values.available()) {
            String h = values.iterate();

            int hyphen = h.indexOf("-", 1);

            if ((hyphen >= 0) && (hyphen < h.length()-1)) {
                int start = h.substring(0, hyphen).toInt();
                int end   = h.substring(hyphen+1).toInt();

                if (start < 0) start = 0;

                for (int i = start; i<=end; ++i) {
                    list.push(String(i));
                }
            } else if ((h.charAt(0) >= '0') && (h.charAt(0) <= '9')) {
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

        Command cmd_welcome = cli.addCommand("welcome", [](cmd* c) {
            debuglnF(VERSION);

            debuglnF("\r\n"
                     "[ ================== DISCLAIMER ================== ]\r\n"
                     "  This is a tool.\r\n"
                     "  It's neither good nor bad.\r\n"
                     "  Use it to study and test.\r\n"
                     "  Never use it to create harm or damage!\r\n"
                     "\r\n"
                     "  The continuation of this project counts on you!\r\n"
                     "[ ================================================ ]\r\n");

            debuglnF("Type \"help\" to see all commands.\r\n"
                     "Type \"start\" to go through the functionalities step by step.");
        });

        cmd_welcome.setDescription("  Print welcome screen including version and disclaimer");

        cmd_welcome.run();

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
            Command command { c };

            String res { command.getArg("cmd").getValue() };
            String cmd;

            debuglnF("Good morning friend!");
            debugln();

            { // Command
                while (!(res == "scan" || res == "beacon" || res == "deauth" || res == "probe" || res == "alias" || res == "results")) {
                    CLI_READ_RES("What can I do for you today?\r\n"
                                 "  scan:    Search for WiFi networks and clients\r\n"
                                 "  ---------\r\n"
                                 "  beacon:  Send WiFi network advertisement beacons (spam network scanners)\r\n"
                                 "  deauth:  Disrupt WiFi connections\r\n"
                                 "  probe:   Send WiFi network requests (spam client scanners)\r\n"
                                 "  ---------\r\n"
                                 "  alias:   Give MAC addresses an alias\r\n"
                                 "  results: Display and filter scan results\r\n"
                                 "Remember that you can always escape by typing 'stop'");
                }
                cmd += res;
            }

            if (res == "scan") {
                { // Scan mode
                    CLI_READ_DEFAULT_UNTIL("What do you wish to scan for?\r\n"
                                           "  ap:    Access points (WiFi networks)\r\n"
                                           "  st:    Stations (WiFi client devices)\r\n"
                                           "  ap+st: Access points and stations",
                                           "ap+st",
                                           (res == "ap" || res == "st" || res == "ap+st"));
                    if (cmd != "ap+st") cmd += " -m " + res;
                }

                // Scan time and channel(s)
                if (res != "ap") {
                    { // Scan time
                        CLI_READ_DEFAULT_UNTIL("Scan for how long?\r\n"
                                               "  >1: Station scan time",
                                               "20",
                                               (res.toInt() > 0));
                        if (res != "20") cmd += " -t " + res;
                    }

                    { // Scan on channel(s)
                        CLI_READ_RES_DEFAULT("Scan on which channel(s)?\r\n"
                                             "  1-14: WiFi channel(s) to search on (for example: 1,6,11)",
                                             "all");
                        if ((res != "all") && (res != "1-14")) cmd += " -ch " + res;
                    }

                    { // Channel scan time
                        CLI_READ_DEFAULT_UNTIL("Stay on each channel for how long?\r\n"
                                               "  >1: Channel time in milliseconds",
                                               "284",
                                               (res.toInt() > 0));
                        if (res != "284") cmd += " -ct " + res;
                    }
                }

                { // Retain scan results
                    CLI_READ_DEFAULT_UNTIL("Keep previous scan results?\r\n"
                                           "  y: Yes\r\n"
                                           "  n: No",
                                           "n",
                                           (res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -r";
                }
            } else if (res == "beacon") {
                { // SSIDs
                    CLI_READ_RES("Which network names do you wish to advertise?\r\n"
                                 "  for example: \"network A\",\"network B\"");
                    cmd += " -ssid " + strh::escape(res);
                }

                { // BSSID
                    CLI_READ_DEFAULT_UNTIL("What is the transmitter or sender address? (BSSID)\r\n"
                                           "  MAC address: for example '00:20:91:aa:bb:5c\r\n"
                                           "  random:      generate random MAC address",
                                           "random",
                                           (res.length() == 17 || res == "random"));
                    if (res != "random") cmd += " -from " + res;
                }

                { // Receiver
                    CLI_READ_DEFAULT_UNTIL("What is the receiver address?\r\n"
                                           "  MAC address: for example 00:20:91:aa:bb:5cc\r\n"
                                           "  broadcast:   send to everyone",
                                           "broadcast",
                                           (res.length() == 17 || res == "broadcast"));
                    if (res != "broadcast") cmd += " -to " + res;
                }

                { // Encryption
                    CLI_READ_DEFAULT_UNTIL("What encryption should it use?\r\n"
                                           "  open: no encryption, an open network without a password\r\n"
                                           "  wpa2: WPA2 protected network",
                                           "open",
                                           (res == "open" || res == "wpa2"));
                    if (res != "open") cmd += " -enc " + res;
                }

                { // Channels
                    CLI_READ_DEFAULT_UNTIL("Which channel(s) should be used?\r\n"
                                           "  1-14: WiFi channel to send packets on",
                                           "1",
                                           (res.toInt() >= 1 && res.toInt() <= 14));
                    if (res != "1") cmd += " -ch " + res;
                }

                { // Time
                    CLI_READ_DEFAULT_UNTIL("How long should the attack last?\r\n"
                                           "   0: Infinite\r\n"
                                           "  >0: Stop after x seconds",
                                           "300",
                                           (res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                }

                { // Scan
                    CLI_READ_DEFAULT_UNTIL("Scan for authentication requests?\r\n"
                                           "  y: Yes\r\n"
                                           "  n: No",
                                           "n",
                                           (res == String('y') || res == String('n')));
                    if (res == String('y')) cmd += " -auth";
                }
            } else if (res == "deauth") {
                { // Target
                    CLI_READ_UNTIL("What do you want to deauthenticate?\r\n"
                                   "  ap:  a network\r\n"
                                   "  st:  a client device\r\n"
                                   "  mac: enter MAC addresses manually",
                                   (res == "ap" || res == "st" || res == "mac"));
                    cmd += " -"+res;

                    if (res == "ap") {
                        if (scan::getAccessPoints().size() == 0) {
                            debuglnF("ERROR: No access points in scan results.\r\n"
                                     "Type 'scan -m ap' to search for access points");
                            return;
                        }

                        result_filter_t filter;

                        filter.channels = 0x3FFF;
                        filter.aps      = true;
                        filter.sts      = false;

                        scan::print(&filter);

                        CLI_READ_RES("Select access point(s) to attack\r\n"
                                     "  >=0: ID(s) to select for the attack");
                        cmd += " "+res;
                    } else if (res == "st") {
                        if (scan::getStations().size() == 0) {
                            debuglnF("ERROR: No stations in scan results.\r\n"
                                     "Type 'scan -m st' to search for stations");
                            return;
                        }

                        result_filter_t filter;

                        filter.channels = 0x3FFF;
                        filter.aps      = false;
                        filter.sts      = true;

                        scan::print(&filter);

                        CLI_READ_RES("Select station(s) to attack\r\n"
                                     "  >=0: ID(s) to select for the attack");

                        cmd += " "+res;
                    } else if (res == "mac") {
                        CLI_READ_RES("Target(s) to attack\r\n"
                                     "  Sender-Receiver-Channel for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'");

                        cmd += " "+res;
                    }
                }

                { // Noob filer
                    CLI_READ_UNTIL("Do you own or have permission to attack the selected devices?\r\n"
                                   "  yes\r\n"
                                   "  no",
                                   (res == "yes" || res == "no"));
                    if (res == "no") {
                        debuglnF("Then you should not attack them.\r\n"
                                 "It could get you in serious trouble.\r\n"
                                 "Please use this tool respectfully!");
                        return;
                    }
                }

                { // Time
                    CLI_READ_DEFAULT_UNTIL("How long should the attack last?\r\n"
                                           "   0: Infinite\r\n"
                                           "  >0: Stop after x seconds",
                                           "300",
                                           (res.toInt() >= 0));
                    if (res != "300") cmd += " -t " + res;
                }

                { // Number of packets
                    CLI_READ_DEFAULT_UNTIL("How many packets shall be sent?\r\n"
                                           "   0: Infinite\r\n"
                                           "  >0: Send x packets",
                                           "0",
                                           (res.toInt() >= 0));
                    if (res != "0") cmd += " -n " + res;
                }

                { // Packet rate
                    CLI_READ_DEFAULT_UNTIL("At which speed/rate?\r\n"
                                           "  >0 : Packets per second",
                                           "20",
                                           (res.toInt() > 0));
                    if (res != "20") cmd += " -r " + res;
                }

                { // Mode
                    CLI_READ_DEFAULT_UNTIL("What kind of packets shall be sent?\r\n"
                                           "  deauth:          Deauthentication\r\n"
                                           "  disassoc:        Disassociation\r\n"
                                           "  deauth+disassoc: Both",
                                           "deauth+disassoc",
                                           (res == "deauth" || res == "disassoc" || res == "deauth+disassoc"));
                    if (res != "deauth+disassoc") cmd += " -m " + res;
                }
            } else if (res == "probe") {
                { // SSIDs
                    CLI_READ_RES("Which network names do you wish to request for?\r\n"
                                 "  for example: \"network A\",\"network B\"");
                    cmd += " -ssid " + strh::escape(res);
                }

                { // Receiver
                    CLI_READ_DEFAULT_UNTIL("What is the receiver address?\r\n"
                                           "  MAC address: for example 00:20:91:aa:bb:5c\r\n"
                                           "  broadcast:   send to everyone",
                                           "broadcast",
                                           (res.length() == 17 || res == "broadcast"));
                    if (res != "broadcast") cmd += " -to " + res;
                }

                { // Channel
                    CLI_READ_DEFAULT_UNTIL("Which channel(s) should be used?\r\n"
                                           "  1-14: WiFi channel to send packets on",
                                           "1",
                                           (res.toInt() >= 1 && res.toInt() <= 14));
                    if (res != String(1)) cmd += " -ch " + res;
                }

                { // Time
                    CLI_READ_DEFAULT_UNTIL("How long should the attack last ?\r\n"
                                           "   0: Infinite\r\n"
                                           "  >0: Stop after x seconds",
                                           "300",
                                           (res.toInt() >= 0));
                    if (res != String(300)) cmd += " -t " + res;
                }
            } else if (res == "alias") {
                { // Mode
                    CLI_READ_DEFAULT_UNTIL("Do you want to set a new alias, remove one or see the entire list ?\r\n"
                                           "  list:   Print list of existing MAC address aliases\r\n"
                                           "  add:    Add new MAC address alias to list\r\n"
                                           "  remove: Remove an exisiting MAC address alias\r\n"
                                           "  clear:  Remove all saved aliases",
                                           "list",
                                           (res == "list" || res == "add" || res == "remove" || res == "clear"));
                    if (res != "list") cmd += " -mode " + res;
                }

                // Add
                if (res == "add") {
                    CLI_READ_DEFAULT_UNTIL("Selector:\r\n"
                                           "  mac: Enter MAC address manually\r\n"
                                           "  ap:  Use the BSSID of an access point\r\n"
                                           "  st:  Use the MAC of a station",
                                           "mac",
                                           (res == "mac" || res == "ap" || res == "st"));

                    { // MAC
                        if (res == "mac") {
                            CLI_READ_UNTIL("Enter a valid MAC address:",
                                           (mac::valid(res.c_str(), res.length())));
                            cmd += " -mac " + res.substring(0, 17);
                        }

                        else if (res == "ap") {
                            if (scan::getAccessPoints().size() == 0) {
                                debuglnF("ERROR: No access points in scan results.\r\n"
                                         "Type 'scan -m ap' to search for access points");
                                return;
                            }

                            result_filter_t filter;

                            filter.channels = 0x3FFF;
                            filter.aps      = false;
                            filter.sts      = true;

                            scan::print(&filter);

                            CLI_READ_UNTIL("Enter access point ID:",
                                           (res.toInt() >= 0));
                            cmd += " -ap " + res;
                        }

                        else if (res == "st") {
                            if (scan::getStations().size() == 0) {
                                debuglnF("ERROR: No stations in scan results.\r\n"
                                         "Type 'scan -m st' to search for stations");
                                return;
                            }

                            result_filter_t filter;

                            filter.channels = 0x3FFF;
                            filter.aps      = false;
                            filter.sts      = true;

                            scan::print(&filter);

                            CLI_READ_UNTIL("Enter station ID:",
                                           (res.toInt() >= 0));
                            cmd += " -st " + res;
                        }
                    }

                    { // Name
                        CLI_READ_RES("Alias name:");
                        if (res.length() > 0) cmd += " -name " + strh::escape(res);
                    }
                }

                // Remove
                else if (res == "remove") {
                    alias::print();
                    CLI_READ_RES("Enter ID, name or MAC address of alias you want to remove:");
                    cmd += " " + res;
                }
            } else if (res == "results") {
                { // Type
                    CLI_READ_DEFAULT_UNTIL("Filter type of results:\r\n"
                                           "  ap:    Access points(WiFi networks)\r\n"
                                           "  st:    Stations(WiFi client devices)\r\n"
                                           "  ap+st: Access points and stations",
                                           "ap+st",
                                           (res == "ap" || res == "st" || res == "ap+st"));
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
                    CLI_READ_RES_DEFAULT("Filter channel(s):\r\n"
                                         "  1-14: WiFi channel(s) to search on(for example: 1, 6, 11)",
                                         "all");
                    if ((res != "all ") && (res != " 1-14 ")) cmd += " -ch " + res;
                }

                { // SSIDs
                    CLI_READ_RES_DEFAULT("Filter for SSIDs(network names):\r\n"
                                         "  for example: \"network A\",\"network B\"",
                                         "");
                    if (res.length() > 0) cmd += " -ssid " + strh::escape(res);
                }

                { // BSSID
                    CLI_READ_RES_DEFAULT("Filter for BSSID (MAC address of access point):\r\n"
                                         "  for example 00:20:91:aa:bb:5cc",
                                         "");
                    if (res.length() > 0) cmd += " -bssid " + res;
                }

                { // Vendor
                    CLI_READ_RES_DEFAULT("Filter for vendor name:\r\n"
                                         "  for example \"Apple\",\"Intel\"",
                                         "");
                    if (res.length() > 0) cmd += " -vendor " + strh::escape(res);
                }
            }


            // Result
            debuglnF("> Exiting start command");

            debugln();
            debugF("# ");
            debugln(cmd);
            debugln();

            cli::parse(cmd.c_str());
        });

        cmd_start.addPosArg("cmd", "");
        cmd_start.setDescription("  Start a guided tour through the functions of this device");

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            bool ap;
            bool st;

            ap_scan_settings_t scan_settings;

            { // Station scan time
                String time_str                   = cmd.getArg("t").getValue();
                scan_settings.st_settings.timeout = parse_time(time_str, 1000);
            }

            { // Channels
                String ch_str          = cmd.getArg("ch").getValue();
                scan_settings.channels = scan_settings.st_settings.channels = parse_channels(ch_str);
            }

            { // Channel scan time
                String time_str                   = cmd.getArg("ct").getValue();
                scan_settings.st_settings.ch_time = parse_time(time_str, 1);
            }

            { // Retain results
                scan_settings.retain = scan_settings.st_settings.retain = cmd.getArg("r").isSet();
            }

            { // Mode
                String mode = cmd.getArg("m").getValue();

                ap = false;
                st = false;

                if (mode == "ap+st") {
                    ap = true;
                    st = true;
                } else if (mode == "ap") {
                    ap = true;
                } else if (mode == "st") {
                    st = true;
                }
            }

            scan_settings.st = st;

            if (ap) scan::startAP(scan_settings);
            else if (st) scan::startST(scan_settings.st_settings);
        });

        cmd_scan.addPosArg("m/ode", "ap+st");
        cmd_scan.addArg("t/ime", "20s");
        cmd_scan.addArg("ch/annel", "all");
        cmd_scan.addArg("ct/ime", "284");
        cmd_scan.addFlagArg("r/etain");
        cmd_scan.setDescription(
            "  Scan for WiFi devices\r\n"
            "  -m:  scan mode [ap,st,ap+st] (default=ap+st)\r\n"
            "  -t:  station scan time (default=20s)\r\n"
            "  -ch: 2.4 GHz channels for station scan [1-14] (default=all)\r\n"
            "  -ct: channel scan time in milliseconds (default=284)\r\n"
            "  -r:  keep previous scan results"
            );

        Command cmd_auth = cli.addCommand("auth", [](cmd* c) {
            Command cmd(c);

            auth_scan_settings_t auth_settings;
            auth_settings.beacon = false;

            { // Channels
                String ch_str          = cmd.getArg("ch").getValue();
                auth_settings.channels = parse_channels(ch_str);
            }

            { // Read Access Point MACs
                String ap_str         = cmd.getArg("ap").getValue();
                SortedStringList list = parse_int_list(ap_str);

                if (list.size() > 0) {
                    auth_settings.channels = 0;
                    MacArr ap_bssids { list.size() };

                    list.begin();

                    while (list.available()) {
                        int id          = list.iterate().toInt();
                        AccessPoint* ap = scan::getAccessPoints().get(id);

                        if (ap) {
                            ap_bssids.add(ap->getBSSID());
                            auth_settings.channels |= 1 << (ap->getChannel()-1);
                        }
                    }

                    auth_settings.bssids += ap_bssids;
                }
            }

            { // Read BSSIDs
                String bssid_str { cmd.getArg("bssid").getValue() };
                auth_settings.bssids += MacArr { bssid_str };
            }

            { // Timeout
                String time_str       = cmd.getArg("t").getValue();
                auth_settings.timeout = parse_time(time_str, 1000);
            }

            { // Channel scan time
                String time_str       = cmd.getArg("ct").getValue();
                auth_settings.ch_time = parse_time(time_str, 1);
            }

            { // Save
                auth_settings.save = cmd.getArg("save").isSet();
            }

            scan::startAuth(auth_settings);
        });

        cmd_auth.addPosArg("bssid", "");
        cmd_auth.addArg("ap", "");
        cmd_auth.addArg("t/ime", "0");
        cmd_auth.addArg("ch/annel", "all");
        cmd_auth.addArg("ct/ime", "284");
        cmd_auth.addFlagArg("save");
        cmd_auth.setDescription("  Authentication scan\r\n"
                                "  -bssid: filter by BSSID(s)\r\n"
                                "  -ap:    filter by access point ID(s)\r\n"
                                "  -ch:    2.4 GHz channels for auth. scan [1-14] (default=all)\r\n"
                                "  -ct:    channel scan time in milliseconds (default=284)\r\n"
                                "  -t:     scan timeout (default=none)\r\n"
                                "  -save:  save recorded probe requests");

        Command cmd_rssi = cli.addCommand("rssi", [](cmd* c) {
            Command cmd(c);

            rssi_scan_settings_t rssi_settings;
            rssi_settings.channels = 0;

            { // MACs
                String bssid_str { cmd.getArg("mac").getValue() };
                rssi_settings.macs += MacArr { bssid_str };
            }

            { // Read Access Point MACs
                String ap_str { cmd.getArg("ap").getValue() };
                SortedStringList list { ap_str };
                AccessPointList& aps = scan::getAccessPoints();
                MacArr ap_bssids { list.size() };

                list.begin();

                while (list.available()) {
                    String value { list.iterate() };
                    SortedStringList ids { parse_int_list(value) };
                    uint8_t mac[6];
                    AccessPoint* ap;

                    if (alias::resolve(value, mac)) { // MAC address (or alias)
                        ap = aps.search(mac);
                        if (!ap) {
                            debugF("WARNING: Access Point ");
                            debug(value);
                            debuglnF(" not found");
                        } else {
                            ap_bssids.add(ap->getBSSID());
                            rssi_settings.channels |= 1 << (ap->getChannel()-1);
                        }
                    } else if (ids.size() > 0) { // ID(s)
                        int id;
                        ids.begin();

                        while (ids.available()) {
                            id = ids.iterate().toInt();
                            ap = aps.get(id);
                            if (!ap) {
                                debugF("WARNING: Access Point ");
                                debug(id);
                                debuglnF(" not found");
                            } else {
                                ap_bssids.add(ap->getBSSID());
                                rssi_settings.channels |= 1 << (ap->getChannel()-1);
                            }
                        }
                    } else { // SSID
                        aps.begin();

                        while (aps.available()) {
                            ap = aps.iterate();
                            if (ap && (String(ap->getSSID()) == value)) {
                                ap_bssids.add(ap->getBSSID());
                                rssi_settings.channels |= 1 << (ap->getChannel()-1);
                            }
                        }
                    }
                }

                rssi_settings.macs += ap_bssids;
            }

            { // Read Station MACs
                String st_str { cmd.getArg("st").getValue() };
                SortedStringList list { st_str };
                StationList& sts = scan::getStations();
                MacArr st_macs { list.size() };

                list.begin();

                while (list.available()) {
                    String value { list.iterate() };
                    SortedStringList ids { parse_int_list(value) };
                    uint8_t mac[6];
                    Station* st;

                    if (alias::resolve(value, mac)) { // MAC address (or alias)
                        st = sts.search(mac);
                        if (!st) {
                            debugF("WARNING: Station ");
                            debug(value);
                            debuglnF(" not found");
                        } else {
                            st_macs.add(st->getMAC());
                            if (st->getAccessPoint()) rssi_settings.channels |= 1 << (st->getAccessPoint()->getChannel()-1);
                        }
                    } else if (ids.size() > 0) { // ID(s)
                        int id;
                        ids.begin();

                        while (ids.available()) {
                            id = ids.iterate().toInt();
                            st = sts.get(id);
                            if (!st) {
                                debugF("WARNING: Station ");
                                debug(id);
                                debuglnF(" not found");
                            } else {
                                if (st->getAccessPoint()) rssi_settings.channels |= 1 << (st->getAccessPoint()->getChannel()-1);
                            }
                        }
                    }
                }

                rssi_settings.macs += st_macs;
            }

            { // Channels
                Argument ch_arg = cmd.getArg("ch");
                if ((rssi_settings.channels == 0) || ch_arg.isSet()) {
                    String ch_str          = ch_arg.getValue();
                    rssi_settings.channels = parse_channels(ch_str);
                }
            }

            { // Channel scan time
                String time_str       = cmd.getArg("ct").getValue();
                rssi_settings.ch_time = parse_time(time_str, 1);
            }

            { // Update Time
                String time_str           = cmd.getArg("ut").getValue();
                rssi_settings.update_time = parse_time(time_str, 1000);
            }

            scan::startRSSI(rssi_settings);
        });

        cmd_rssi.addPosArg("mac", "");
        cmd_rssi.addArg("ap", "");
        cmd_rssi.addArg("st/ation", "");
        cmd_rssi.addArg("ch/annel", "all");
        cmd_rssi.addArg("ct/ime", "120");
        cmd_rssi.addArg("ut,u/pdate/time", "1s");
        cmd_rssi.setDescription("  Signal Strength scan\r\n"
                                "  -mac: filter by MAC(s)\r\n"
                                "  -ap:  filter by AP(s)\r\n"
                                "  -st:  filter by Station(s)\r\n"
                                "  -ch:  2.4 GHz channel(s) for scan [1-14] (default=all)\r\n"
                                "  -ct:  channel scan time in milliseconds (default=120)\r\n"
                                "  -ut:  update time (default=1s)");

        Command cmd_results = cli.addCommand("results", [](cmd* c) {
            Command cmd(c);
            String mode = cmd.getArg("t").getValue();

            String ch_str     = cmd.getArg("ch").getValue();
            String ssid_str   = cmd.getArg("ssid").getValue();
            String bssid_str  = cmd.getArg("bssid").getValue();
            String vendor_str = cmd.getArg("vendor").getValue();

            uint16_t channels = parse_channels(ch_str);
            StringList vendors { vendor_str };

            result_filter_t filter;

            filter.channels = channels;
            filter.ssids.parse(ssid_str);
            filter.bssids.parse(bssid_str);
            filter.vendors.parse(vendor_str);
            filter.aps = (mode == "ap") || (mode == "ap+st");
            filter.sts = (mode == "st") || (mode == "ap+st");

            scan::print(&filter);
        });

        cmd_results.addPosArg("t/ype", "ap+st");
        cmd_results.addArg("ch/annel/s", "all");
        cmd_results.addArg("ssid/s", "");
        cmd_results.addArg("bssid/s", "");
        cmd_results.addArg("vendor/s", "");
        cmd_results.setDescription(
            "  Print list of scan results [access points (networks) and stations (clients)]\r\n"
            "  -type:   type of results [ap,st,ap+st] (default=ap+st)\r\n"
            "  -ch:     filter by channel(s)\r\n"
            "  -ssid:   filter by SSID(s)\r\n"
            "  -bssid:  filter by BSSID(s)\r\n"
            "  -vendor: filter by vendor name(s)");

        Command cmd_beacon = cli.addCommand("beacon", [](cmd* c) {
            Command cmd(c);

            bool scan;

            beacon_attack_settings_t beacon_settings;

            { // SSIDs
                String ssids { cmd.getArg("ssid").getValue() };
                beacon_settings.ssids.parse(ssids);
            }

            { // BSSID
                String bssid_str { cmd.getArg("from").getValue() };
                parse_mac(bssid_str, beacon_settings.bssid);
            }

            { // Receiver
                String receiver_str { cmd.getArg("to").getValue() };
                parse_mac(receiver_str, beacon_settings.receiver);
            }

            { // Encryption
                String enc_str { cmd.getArg("enc").getValue() };
                if (enc_str == "wpa2") beacon_settings.enc = ENCRYPTION_WPA2;
                else beacon_settings.enc = ENCRYPTION_OPEN;
            }

            { // Channels
                String ch_str { cmd.getArg("ch").getValue() };
                beacon_settings.channels = parse_channels(ch_str);
            }

            { // Packet rate
                String pkt_rate_str { cmd.getArg("r").getValue() };
                beacon_settings.pkt_rate = pkt_rate_str.toInt();
            }

            { // Time
                String time_str { cmd.getArg("t").getValue() };
                beacon_settings.timeout = parse_time(time_str, 1000);
            }

            { // Monitor connection tries
                beacon_settings.scan = cmd.getArg("m").isSet();
            }

            { // Save
                beacon_settings.save_scan = cmd.getArg("save").isSet();
            }

            attack::startBeacon(beacon_settings);
        });

        cmd_beacon.addPosArg("ssid/s");
        cmd_beacon.addArg("bssid,from", "random");
        cmd_beacon.addArg("receiver,to", "broadcast");
        cmd_beacon.addPosArg("enc/ryption", "open");
        cmd_beacon.addArg("ch/annel", "1");
        cmd_beacon.addArg("r/ate", "10");
        cmd_beacon.addFlagArg("auth,m/on/itor");
        cmd_beacon.addFlagArg("save");
        cmd_beacon.addArg("t/ime/out", "5min");
        cmd_beacon.setDescription(
            "  Send WiFi network advertisement beacons\r\n"
            "  -ssid: network names (SSIDs) for example: \"test A\",\"test B\"\r\n"
            "  -from: BSSID or sender MAC address (default=random)\r\n"
            "  -to:   receiver MAC address (default=broadcast)\r\n"
            "  -enc:  encryption [open,wpa2] (default=open)\r\n"
            "  -ch:   2.4 GHz channel(s) [1-14] (default=1)\r\n"
            "  -r:    packets per second per SSID (default=10)\r\n"
            "  -m:    scan for authentications\r\n"
            "  -save: save probe requests from auth. scan\r\n"
            "  -t:    attack timeout (default=5min)"
            );

        Command cmd_deauth = cli.addCommand("deauth", [](cmd* c) {
            Command cmd(c);

            deauth_attack_settings_t deauth_settings;

            { // Read Access Point MACs
                String ap_str { cmd.getArg("ap").getValue() };
                SortedStringList list { ap_str };
                AccessPointList& aps = scan::getAccessPoints();
                TargetArr ap_targets { aps.size() };

                list.begin();

                while (list.available()) {
                    String value { list.iterate() };
                    SortedStringList ids { parse_int_list(value) };
                    uint8_t mac[6];
                    AccessPoint* ap;

                    if (alias::resolve(value, mac)) { // MAC address (or alias)
                        ap = aps.search(mac);
                        ap_targets.add(ap);
                    } else if (ids.size() > 0) {      // ID(s)
                        int id;
                        ids.begin();

                        while (ids.available()) {
                            id = ids.iterate().toInt();
                            ap = aps.get(id);
                            ap_targets.add(ap);
                        }
                    } else { // SSID
                        aps.begin();

                        while (aps.available()) {
                            ap = aps.iterate();
                            if (String(ap->getSSID()) == value) {
                                ap_targets.add(ap);
                            }
                        }
                    }
                }

                deauth_settings.targets += ap_targets;
            }

            { // Read Station MACs
                String st_str { cmd.getArg("st").getValue() };
                SortedStringList list { st_str };
                StationList& sts = scan::getStations();
                TargetArr st_targets { sts.size() };

                list.begin();

                while (list.available()) {
                    String value { list.iterate() };
                    SortedStringList ids { parse_int_list(value) };
                    uint8_t mac[6];
                    Station* st;

                    if (alias::resolve(value, mac)) { // MAC address (or alias)
                        st = sts.search(mac);
                        if (!st_targets.add(st)) {
                            debugF("WARNING: Station ");
                            debug(value);
                            debuglnF(" not found, not connected to an AP or already in list.");
                        }
                    } else if (ids.size() > 0) { // ID(s)
                        int id;
                        ids.begin();

                        while (ids.available()) {
                            id = ids.iterate().toInt();
                            st = sts.get(id);
                            if (!st_targets.add(st)) {
                                debugF("WARNING: Station ");
                                debug(id);
                                debuglnF(" not found, not connected to an AP or already in list.");
                            }
                        }
                    }
                }

                deauth_settings.targets += st_targets;
            }

            { // Read custom MACs
                String mac_str { cmd.getArg("mac").getValue() };
                StringList list { mac_str };

                TargetArr manual_targets { list.size() };

                list.begin();

                while (list.available()) {
                    String target = list.iterate();
                    StringList target_data(target, "-");

                    if (target_data.size() != 3) continue;

                    String sender_mac_str   { target_data.iterate() };
                    String receiver_mac_str { target_data.iterate() };
                    String ch_str           { target_data.iterate() };

                    uint8_t sender[6];
                    uint8_t receiver[6];
                    uint16_t channels = parse_channels(ch_str);

                    parse_mac(sender_mac_str, sender);
                    parse_mac(receiver_mac_str, receiver);

                    manual_targets.add(sender, receiver, channels);
                }

                deauth_settings.targets += manual_targets;
            }

            { // Time
                String time_str { cmd.getArg("t").getValue() };
                deauth_settings.timeout = parse_time(time_str, 1000);
            }

            { // Number
                deauth_settings.max_pkts = cmd.getArg("n").getValue().toInt();
            }

            { // Rate
                deauth_settings.pkt_rate = cmd.getArg("r").getValue().toInt();
            }

            { // Mode
                String mode { cmd.getArg("m").getValue() };

                if (mode == "deauth+disassoc") {
                    deauth_settings.deauth   = true;
                    deauth_settings.disassoc = true;
                } else if (mode == "deauth") {
                    deauth_settings.deauth   = true;
                    deauth_settings.disassoc = false;
                } else if (mode == "disassoc") {
                    deauth_settings.deauth   = false;
                    deauth_settings.disassoc = true;
                }
            }

            attack::startDeauth(deauth_settings);
        });

        cmd_deauth.addArg("ap", "");
        cmd_deauth.addArg("st/ation", "");
        cmd_deauth.addArg("mac,manual", "");
        cmd_deauth.addArg("t/ime/out", "5min");
        cmd_deauth.addArg("n/um/ber", "0");
        cmd_deauth.addArg("r/ate", "20");
        cmd_deauth.addArg("m/ode", "deauth+disassoc");
        cmd_deauth.setDescription(
            "  Deauthenticate (disconnect) selected WiFi connections\r\n"
            "  -ap:  access point IDs to attack\r\n"
            "  -st:  station IDs to attack\r\n"
            "  -mac: manual target selection [Sender-Receiver-Channel] for example:'aa:bb:cc:dd:ee:ff-00:11:22:33:44:55-7'\r\n"
            "  -t:   attack timeout (default=5min)\r\n"
            "  -n:   packet limit [>1] (default=0)\r\n"
            "  -r:   packets per second (default=20)\r\n"
            "  -m:   packet types [deauth,disassoc,deauth+disassoc] (default=deauth+disassoc)"
            );

        Command cmd_probe = cli.addCommand("probe", [](cmd* c) {
            Command cmd(c);

            probe_attack_settings_t probe_settings;

            { // SSIDs
                String ssids { cmd.getArg("ssid").getValue() };
                probe_settings.ssids.parse(ssids);
            }

            { // Sender
                String sender { cmd.getArg("from").getValue() };
                parse_mac(sender, probe_settings.sender);
            }

            { // Receiver
                String receiver_str { cmd.getArg("to").getValue() };
                parse_mac(receiver_str, probe_settings.receiver);
            }

            { // Channel
                String ch_str {  cmd.getArg("ch").getValue() };
                probe_settings.channels = parse_channels(ch_str);
            }

            { // Packet rate
                probe_settings.pkt_rate = cmd.getArg("r").getValue().toInt();
            }

            { // Time
                String time_str { cmd.getArg("t").getValue() };
                probe_settings.timeout = parse_time(time_str, 1000);
            }

            attack::startProbe(probe_settings);
        });

        cmd_probe.addPosArg("ssid/s");
        cmd_probe.addArg("sender,from", "random");
        cmd_probe.addArg("receiver,to", "broadcast");
        cmd_probe.addArg("ch/annel/s", "1");
        cmd_probe.addArg("r/ate", "10");
        cmd_probe.addArg("t/ime/out", "5min");
        cmd_probe.setDescription(
            "  Send probe requests for WiFi networks\r\n"
            "  -ssid: network names (SSIDs) for example: \"test A\",\"test B\"\r\n"
            "  -from: sender MAC address (default=random)\r\n"
            "  -to:   receiver MAC address (default=broadcast)\r\n"
            "  -ch:   2.4 GHz channel(s) [1-14] (default=1)\r\n"
            "  -r:    packets per second per SSID (default=10)\r\n"
            "  -t:    attack timeout (default=5min)"
            );

        Command cmd_alias = cli.addCommand("alias", [](cmd* c) {
            Command cmd(c);

            Argument arg_mac { cmd.getArg("mac") };
            Argument arg_ap { cmd.getArg("ap") };
            Argument arg_st { cmd.getArg("st") };

            String mode { cmd.getArg("mode").getValue() };
            String name { cmd.getArg("name").getValue() };
            String mac_str;

            if (arg_mac.isSet()) {
                mac_str = arg_mac.getValue();
            } else if (arg_ap.isSet()) {
                int id { arg_ap.getValue().toInt() };
                AccessPoint* ap { scan::getAccessPoints().get(id) };
                if (ap) mac_str = ap->getBSSIDString();
            } else if (arg_st.isSet()) {
                int id = arg_st.getValue().toInt();
                Station* st { scan::getStations().get(id) };
                if (st) mac_str = st->getMACString();
            }

            if (mode == "list") {
                alias::print();
                return;
            }

            else if (mode == "add") {
                // No valid mac? Try switching arg values!
                if (!mac::valid(mac_str.c_str(), mac_str.length())) {
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
                    debuglnF("Name or address already in list or invalid");
                }
                return;
            }

            else if (mode == "remove") {
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

            else if (mode == "clear") {
                alias::clear();
                debuglnF("Cleared alias list");
            }

            else {
                debugF("Unknown mode \"");
                debug(mode);
                debuglnF("\".");
                debuglnF("Try \"alias list\", \"alias add [...]\", \"alias remove [...]\" or \"alias clear\".");
            }
        });

        cmd_alias.addPosArg("m/ode", "list");
        cmd_alias.addPosArg("name", "");
        cmd_alias.addPosArg("mac", "");
        cmd_alias.addArg("ap", "");
        cmd_alias.addArg("st", "");
        cmd_alias.setDescription(
            "  Manage alias for MAC address\r\n"
            "  -mode: list,add,remove or clear (default=list)\r\n"
            "  -name: alias name\r\n"
            "  -mac:  MAC address\r\n"
            "  -ap:   access point ID\r\n"
            "  -st:   station ID");

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
            Command cmd(c);

            String mode_str { cmd.getArg("mode").getValue() };

            if (mode_str == "all") {
                ap::stop();
                attack::stop();
                scan::stop();
            } else {
                SortedStringList mode_list { mode_str };
                mode_list.begin();

                while (mode_list.available()) {
                    String mode { mode_list.iterate() };
                    if (mode == "scan") {
                        scan::stopAP();
                        scan::stopST();
                    } else if (mode == "auth") {
                        scan::stopAuth();
                    } else if (mode == "rssi") {
                        scan::stopRSSI();
                    } else if (mode == "attack") {
                        attack::stop();
                    } else if (mode == "beacon") {
                        attack::stopBeacon();
                    } else if (mode == "deauth") {
                        attack::stopDeauth();
                    } else if (mode == "probe") {
                        attack::stopProbe();
                    } else if (mode == "ap") {
                        ap::stop();
                    }
                }
            }
        });

        cmd_stop.addPosArg("mode", "all");
        cmd_stop.setDescription("  Stop scans or attacks\r\n"
                                "  -mode: all,scan,auth,rssi,attack,beacon,deauth,probe,ap (default=all)");

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

        Command cmd_vendor = cli.addCommand("vendor", [](cmd* c) {
            Command cmd(c);

            String input { cmd.getArg("mac").getValue() };
            bool exact { cmd.getArg("e").isSet() };

            uint8_t mac[3];

            debuglnF("MAC      Vendor");
            debuglnF("=================");

            if (alias::resolve(input, mac, 3)) {
                debug(strh::mac(mac, 3));
                debug(' ');
                debugln(vendor::getName(mac));
            } else {
                vendor::getMAC(input, !exact, [](const uint8_t* mac, const char* name) {
                    debug(strh::mac(mac, 3));
                    debug(' ');
                    debugln(name);
                });
            }
            debuglnF("=================");
        });
        cmd_vendor.addPosArg("mac");
        cmd_vendor.addFlagArg("e/xact");
        cmd_vendor.setDescription("  Vendor (manufacturer) lookup\r\n"
                                  "  -mac: MAC address(es)\r\n"
                                  "  -e:   list only exact matchess");

        Command cmd_wait = cli.addCommand("wait", [](cmd* c) {
            debugln("> Paused CLI");
            cli.pause();
        });
        cmd_wait.setDescription("  Wait until scan or attack has finished");

        Command cmd_sleep = cli.addCommand("sleep", [](cmd* c) {
            Command cmd(c);

            timer = millis() + parse_time(cmd.getArg("t").getValue(), 1);
            cli.pause();
        });
        cmd_sleep.addPosArg("t/ime", "");
        cmd_sleep.setDescription("  Sleep for specified amount of time\r\n"
                                 "  -t: time to sleep");

        Command cmd_ap = cli.addCommand("ap", [](cmd* c) {
            Command cmd(c);

            String ssid { cmd.getArg("s").getValue() };
            String pswd { cmd.getArg("p").getValue() };
            bool hidden { cmd.getArg("h").isSet() };
            uint8_t channel { (uint8_t)cmd.getArg("ch").getValue().toInt() };
            uint8_t bssid[6];
            String bssid_str { cmd.getArg("bssid").getValue() };
            parse_mac(bssid_str, bssid);

            ap::start(ssid, pswd, hidden, channel, bssid);
        });
        cmd_ap.addPosArg("s/sid");
        cmd_ap.addPosArg("p/assword", "");
        cmd_ap.addFlagArg("hidden");
        cmd_ap.addArg("ch/annel", "1");
        cmd_ap.addArg("b/ssid", "random");
        cmd_ap.setDescription("  Start access point\r\n"
                              "  -s:  SSID network name\r\n"
                              "  -p:  Password with at least 8 characters\r\n"
                              "  -h:  Hidden network\r\n"
                              "  -ch: Channel (default=1)\r\n"
                              "  -b:  BSSID MAC address (default=random)");

        Command cmd_demo = cli.addCommand("demo", [](cmd* c) {
            Command cmd(c);

            String mode { cmd.getArg("m").getValue() };

            if (mode == "on") {
                strh::hide_mac(true);
                debugln("Demo Mode: ACTIVATED");
            } else {
                strh::hide_mac(false);
                debugln("Demo Mode: DEACTIVATED");
            }
        });
        cmd_demo.addPosArg("m/ode", "on");
        cmd_demo.setDescription("  Demo mode\r\n"
                                "  on:  turn on demo mode\r\n"
                                "  off: turn off demo mode");
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
        if (cli.paused() && (((timer == 0) && !scan::active() && !attack::active()) || ((timer > 0) && (millis() > timer)))) {
            debugln("> Resumed CLI");
            cli.unpause();
            timer = 0;
        }

        if (debug_available()) {
            //  String input = debug_read();
            String input;
            read(input);

            if (input.length() > 0) {
                cli::parse(input.c_str());
#ifdef ENABLE_HISTORY
                if (history.full()) {
                    history.begin();
                    history.remove();
                }
                history.push(input);
#endif // ifdef ENABLE_HISTORY
            }
        }
    }
}