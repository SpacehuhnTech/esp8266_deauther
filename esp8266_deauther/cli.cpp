/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "cli.h"

#include <SimpleCLI.h> // SimpleCLI library
#include "debug.h"     // debug(), debugln(), debugf()
#include "scan.h"

// ram usage
extern "C" {
  #include "user_interface.h"
}

namespace cli {
    // ===== PRIVATE ===== //
    SimpleCLI cli; // !< Instance of SimpleCLI library

    // ===== PUBLIC ===== //
    void begin() {
        cli.setOnError([](cmd_error* e) {
            CommandError cmdError(e); // Create wrapper object

            String res = "ERROR: " + cmdError.toString();

            if (cmdError.hasCommand()) {
                res += "\nDid you mean \"";
                res += cmdError.getCommand().toString();
                res += "\"?";
            }

            debugln(res);
        });

        Command cmd_help = cli.addCommand("help", [](cmd* c) {
            debugln(cli.toString());
        });
        cmd_help.setDescription("  Prints the list of commands that you see right now");

        Command cmd_clear = cli.addCommand("clear", [](cmd* c) {
            for (uint8_t i = 0; i<100; ++i) {
                debugln();
            }
        });
        cmd_clear.setDescription("  Clears serial output (by spamming line breaks :P)");

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
        cmd_ram.setDescription("  Prints memory usage");

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            // Parse scan time
            int scan_time = cmd.getArg("t").getValue().toInt();
            if (scan_time < 0) scan_time = -scan_time;
            scan_time *= 1000;

            // Parse channels
            String channels      = cmd.getArg("ch").getValue();
            uint16_t channel_reg = 0;
            String tmp_ch;

            for (int i = 0; i<=channels.length(); ++i) {
                if ((i == channels.length()) || (channels.charAt(i) == ',')) {
                    int channel_num = tmp_ch.toInt();
                    if ((channel_num >= 1) && (channel_num <= 14)) {
                        channel_reg |= 1 << (channel_num-1);
                        tmp_ch       = String();
                    }
                } else {
                    tmp_ch += channels.charAt(i);
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
            "  -m or -mode: scan mode [ap,st,ap+st] (default=ap+st)\n"
            "  -t or -time: station scan time in seconds [>1] (default=14)\n"
            "  -ch or -channel: 2.4 GHz channels for station scan [1-14] (default=all)\n"
            "  -r or -retain: Keep previous scan results"
            );

        Command cmd_results = cli.addCommand("results", [](cmd* c) {
            scan::printResults();
        });
        cmd_results.setDescription("  Prints list of scan results [access points (networks) and stations (clients)]");
    }

    void parse(const char* input) {
        debug("# ");
        debug(input);
        debugln();

        cli.parse(input);
    }
}