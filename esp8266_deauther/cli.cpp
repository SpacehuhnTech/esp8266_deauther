/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "cli.h"

#include <SimpleCLI.h> // SimpleCLI library
#include "debug.h"     // debug(), debugln(), debugf()
#include "scan.h"

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

        cli.addCommand("help", [](cmd* c) {
            debugln(cli.toString());
        });

        Command cmd_scan = cli.addCommand("scan", [](cmd* c) {
            Command cmd(c);

            int scan_time = cmd.getArg("t").getValue().toInt();
            if (scan_time < 0) scan_time = -scan_time;

            scan_time *= 1000;

            scan::searchAPs();
            scan::searchSTs(scan_time);
        });
        cmd_scan.addArg("t", "14");
    }

    void parse(const char* input) {
        debug("# ");
        debug(input);
        debugln();

        cli.parse(input);
    }
}