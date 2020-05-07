#ifndef language_h
#define language_h

#include "Arduino.h"

extern String str(const char* ptr);
extern String keyword(const char* keywordPtr);
extern bool eqls(const char* str, const char* keywordPtr);
extern bool eqls(String str, const char* keywordPtr);
extern String b2s(bool input);
extern String b2a(bool input);
extern bool s2b(String input);
extern void prnt(const String s);
extern void prnt(const bool b);
extern void prnt(const char c);
extern void prnt(const char* ptr);
extern void prnt(const char* ptr, int len);
extern void prnt(const int i);
extern void prnt(const uint32_t i);
extern void prntln();
extern void prntln(const String s);
extern void prntln(const bool b);
extern void prntln(const char c);
extern void prntln(const char* ptr);
extern void prntln(const char* ptr, int len);
extern void prntln(const int i);
extern void prntln(const uint32_t i);

/*
   The following variables are the strings used for the serial interface, display interface and settings.
   The keywords for the serial CLI have a simple structure to save a bit of memory and CPU time:
   - every keyword has a unique string
   - / is used for optional characters, i.e. 'enable/d' makes 'enable' and 'enabled'
   - , is used for seperations, i.e. 'select/ed,-s' makes 'select', 'selected' and '-s'
   - everything is in lowercase
 */

// ===== GLOBAL STRINGS ===== //

// Often used characters, therefor in the RAM
const char CURSOR              = '|';
const char SPACE               = ' ';
const char DOUBLEPOINT         = ':';
const char EQUALS              = '=';
const char HASHSIGN            = '#';
const char ASTERIX             = '*';
const char PERCENT             = '%';
const char DASH                = '-';
const char QUESTIONMARK        = '?';
const char ZERO                = '0';
const char S                   = 's';
const char M                   = 'm';
const char D                   = 'd';
const char DOUBLEQUOTES        = '\"';
const char SLASH               = '/';
const char NEWLINE             = '\n';
const char CARRIAGERETURN      = '\r';
const char SEMICOLON           = ';';
const char BACKSLASH           = '\\';
const char POINT               = '.';
const char VERTICALBAR         = '|';
const char COMMA               = ',';
const char ENDOFLINE           = '\0';
const char OPEN_BRACKET        = '[';
const char CLOSE_BRACKET       = ']';
const char OPEN_CURLY_BRACKET  = '{';
const char CLOSE_CURLY_BRACKET = '}';

const char STR_TRUE[] PROGMEM = "true";
const char STR_FALSE[] PROGMEM = "false";
const char STR_MIN[] PROGMEM = "min";

// ===== SETUP ===== //
const char SETUP_OK[] PROGMEM = "OK";
const char SETUP_ERROR[] PROGMEM = "ERROR";
const char SETUP_MOUNT_SPIFFS[] PROGMEM = "Mounting SPIFFS...";
const char SETUP_FORMAT_SPIFFS[] PROGMEM = "Formatting SPIFFS...";
const char SETUP_FORMAT_EEPROM[] PROGMEM = "Formatting EEPROM...";
const char SETUP_SERIAL_WARNING[] PROGMEM = "Warning: Serial deactivated";
const char SETUP_STARTED[] PROGMEM = "STARTED! \\o/";
const char SETUP_COPYING[] PROGMEM = "Copying ";
const char SETUP_PROGMEM_TO_SPIFFS[] PROGMEM = " from PROGMEM to SPIFFS...";

// ===== SERIAL COMMAND LINE INTERFACE ===== //
const char CLI_SCAN[] PROGMEM = "scan";                        // scan
const char CLI_REBOOT[] PROGMEM = "reboot";                    // reboot
const char CLI_STATUS[] PROGMEM = "status";                    // status
const char CLI_SHOW[] PROGMEM = "show";                        // show
const char CLI_REMOVE[] PROGMEM = "remove";                    // remove
const char CLI_SET[] PROGMEM = "set";                          // set
const char CLI_STOP[] PROGMEM = "stop";                        // stop
const char CLI_LOAD[] PROGMEM = "load";                        // load
const char CLI_SAVE[] PROGMEM = "save";                        // save
const char CLI_ADD[] PROGMEM = "add";                          // add
const char CLI_DESELECT[] PROGMEM = "deselect";                // deselect
const char CLI_CLEAR[] PROGMEM = "clear";                      // clear
const char CLI_SYSINFO[] PROGMEM = "sysinfo";                  // sysinfo
const char CLI_RESET[] PROGMEM = "reset";                      // reset
const char CLI_ON[] PROGMEM = "on";                            // on
const char CLI_OFF[] PROGMEM = "off";                          // off
const char CLI_RANDOM[] PROGMEM = "random";                    // random
const char CLI_GET[] PROGMEM = "get";                          // get
const char CLI_INFO[] PROGMEM = "info";                        // info
const char CLI_HELP[] PROGMEM = "help";                        // help
const char CLI_RICE[] PROGMEM = "rice";                        // rice
const char CLI_FORMAT[] PROGMEM = "format";                    // format
const char CLI_DELETE[] PROGMEM = "delete";                    // delete
const char CLI_PRINT[] PROGMEM = "print";                      // print
const char CLI_RUN[] PROGMEM = "run";                          // run
const char CLI_WRITE[] PROGMEM = "write";                      // write
const char CLI_LED[] PROGMEM = "led";                          // led
const char CLI_SEND[] PROGMEM = "send";                        // send
const char CLI_CUSTOM[] PROGMEM = "custom";                    // custom
const char CLI_DELAY[] PROGMEM = "delay";                      // delay
const char CLI_REPLACE[] PROGMEM = "replace";                  // replace
const char CLI_DRAW[] PROGMEM = "draw";                        // draw
const char CLI_SCRIPT[] PROGMEM = "script";                    // script
const char CLI_STARTAP[] PROGMEM = "startap";                  // startap
const char CLI_STOPAP[] PROGMEM = "stopap";                    // stopap
const char CLI_RENAME[] PROGMEM = "rename";                    // rename
const char CLI_COPY[] PROGMEM = "copy";                        // copy
const char CLI_ENABLE[] PROGMEM = "enable/d";                  // enable, enabled
const char CLI_DISABLE[] PROGMEM = "disable/d";                // disable, disabled
const char CLI_WPA2[] PROGMEM = "wpa/2,-wpa/2";                // wpa, wpa2, -wpa, -wpa2
const char CLI_ATTACK[] PROGMEM = "attack/s";                  // attack, attacks
const char CLI_CHICKEN[] PROGMEM = "chicken/s";                // chicken, chickens
const char CLI_SETTING[] PROGMEM = "setting/s";                // setting, settings
const char CLI_ID[] PROGMEM = "id,-i/d";                       // id, -i, -id
const char CLI_ALL[] PROGMEM = "all,-a";                       // all, -a
const char CLI_TIME[] PROGMEM = "time,-t";                     // time, -t
const char CLI_CONTINUE[] PROGMEM = "continue,-c";             // continue, -c
const char CLI_CHANNEL[] PROGMEM = "channel,-ch";              // channel, -ch
const char CLI_MAC[] PROGMEM = "mac,-m";                       // mac, -m
const char CLI_BSSID[] PROGMEM = "bssid,-b";                   // bssid, -b
const char CLI_BEACON[] PROGMEM = "beacon,-b";                 // bssid, -b
const char CLI_DEAUTH[] PROGMEM = "deauth,-d";                 // deauth, -d
const char CLI_DEAUTHALL[] PROGMEM = "deauthall,-da";          // deauthall, -da
const char CLI_PROBE[] PROGMEM = "probe,-p";                   // probe, -p
const char CLI_NOOUTPUT[] PROGMEM = "nooutput,-no";            // nooutput, -no
const char CLI_FORCE[] PROGMEM = "force,-f";                   // force, -f
const char CLI_TIMEOUT[] PROGMEM = "timeout,-t";               // timeout, -t
const char CLI_WIFI[] PROGMEM = "wifi,-w";                     // wifi, -w
const char CLI_CLONES[] PROGMEM = "clones,-cl";                // clones, -cl
const char CLI_PATH[] PROGMEM = "path,-p";                     // path, -p
const char CLI_PASSWORD[] PROGMEM = "password,-ps/wd";         // password, -ps, -pswd
const char CLI_HIDDEN[] PROGMEM = "hidden,-h";                 // hidden, -h
const char CLI_CAPTIVEPORTAL[] PROGMEM = "captiveportal,-cp";  // captiveportal, -cp
const char CLI_SELECT[] PROGMEM = "select/ed,-s";              // select, selected, -s
const char CLI_SSID[] PROGMEM = "ssid/s,-s/s";                 // ssid, ssids, -s, -ss
const char CLI_AP[] PROGMEM = "ap/s,-ap/s";                    // ap, aps, -ap, -aps
const char CLI_STATION[] PROGMEM = "station/s,-st";            // station, stations, -st
const char CLI_NAME[] PROGMEM = "name/s,-n";                   // name, names, -n
const char CLI_LINE[] PROGMEM = "line/s,-l";                   // line, lines, -l
const char CLI_COMMENT[] PROGMEM = "//";                       // //
const char CLI_SCREEN[] PROGMEM = "screen";                    // screen
const char CLI_MODE[] PROGMEM = "mode,-m";                     // mode
const char CLI_MODE_BUTTONTEST[] PROGMEM = "buttontest";       // buttontest
const char CLI_MODE_PACKETMONITOR[] PROGMEM = "packetmonitor"; // packetmonitor
const char CLI_MODE_LOADINGSCREEN[] PROGMEM = "loadingscreen"; // loading
const char CLI_MODE_MENU[] PROGMEM = "menu";                   // menu

const char CLI_HELP_HELP[] PROGMEM = "help";
const char CLI_HELP_SCAN[] PROGMEM = "scan [<all/aps/stations>] [-t <time>] [-c <continue-time>] [-ch <channel>]";
const char CLI_HELP_SHOW[] PROGMEM = "show [selected] [<all/aps/stations/names/ssids>]";
const char CLI_HELP_SELECT[] PROGMEM = "select [<all/aps/stations/names>] [<id>]";
const char CLI_HELP_DESELECT[] PROGMEM = "deselect [<all/aps/stations/names>] [<id>]";
const char CLI_HELP_SSID_A[] PROGMEM = "add ssid <ssid> [-wpa2] [-cl <clones>]";
const char CLI_HELP_SSID_B[] PROGMEM = "add ssid -ap <id> [-cl <clones>] [-f]";
const char CLI_HELP_SSID_C[] PROGMEM = "add ssid -s [-f]";
const char CLI_HELP_NAME_A[] PROGMEM = "add name <name> [-ap <id>] [-s]";
const char CLI_HELP_NAME_B[] PROGMEM = "add name <name> [-st <id>] [-s]";
const char CLI_HELP_NAME_C[] PROGMEM = "add name <name> [-m <mac>] [-ch <channel>] [-b <bssid>] [-s]";
const char CLI_HELP_SET_NAME[] PROGMEM = "set name <id> <newname>";
const char CLI_HELP_ENABLE_RANDOM[] PROGMEM = "enable random <interval>";
const char CLI_HELP_DISABLE_RANDOM[] PROGMEM = "disable random";
const char CLI_HELP_LOAD[] PROGMEM = "load [<all/ssids/names/settings>] [<file>]";
const char CLI_HELP_SAVE[] PROGMEM = "save [<all/ssids/names/settings>] [<file>]";
const char CLI_HELP_REMOVE_A[] PROGMEM = "remove <ap/station/name/ssid> <id>";
const char CLI_HELP_REMOVE_B[] PROGMEM = "remove <ap/station/names/ssids> [all]";
const char CLI_HELP_ATTACK[] PROGMEM = "attack [beacon] [deauth] [deauthall] [probe] [nooutput] [-t <timeout>]";
const char CLI_HELP_ATTACK_STATUS[] PROGMEM = "attack status [<on/off>]";
const char CLI_HELP_STOP[] PROGMEM = "stop <all/scan/attack/script>";
const char CLI_HELP_SYSINFO[] PROGMEM = "sysinfo";
const char CLI_HELP_CLEAR[] PROGMEM = "clear";
const char CLI_HELP_FORMAT[] PROGMEM = "format";
const char CLI_HELP_PRINT[] PROGMEM = "print <file> [<lines>]";
const char CLI_HELP_DELETE[] PROGMEM = "delete <file> [<lineFrom>] [<lineTo>]";
const char CLI_HELP_REPLACE[] PROGMEM = "replace <file> <line> <new-content>";
const char CLI_HELP_COPY[] PROGMEM = "copy <file> <newfile>";
const char CLI_HELP_RENAME[] PROGMEM = "rename <file> <newfile>";
const char CLI_HELP_RUN[] PROGMEM = "run <file>";
const char CLI_HELP_WRITE[] PROGMEM = "write <file> <commands>";
const char CLI_HELP_GET[] PROGMEM = "get <setting>";
const char CLI_HELP_SET[] PROGMEM = "set <setting> <value>";
const char CLI_HELP_RESET[] PROGMEM = "reset";
const char CLI_HELP_CHICKEN[] PROGMEM = "chicken";
const char CLI_HELP_REBOOT[] PROGMEM = "reboot";
const char CLI_HELP_INFO[] PROGMEM = "info";
const char CLI_HELP_COMMENT[] PROGMEM = "// <comments>";
const char CLI_HELP_SEND_DEAUTH[] PROGMEM = "send deauth <apMac> <stMac> <rason> <channel>";
const char CLI_HELP_SEND_BEACON[] PROGMEM = "send beacon <mac> <ssid> <ch> [wpa2]";
const char CLI_HELP_SEND_PROBE[] PROGMEM = "send probe <mac> <ssid> <ch>";
const char CLI_HELP_LED_A[] PROGMEM = "led <r> <g> <b> [<brightness>]";
const char CLI_HELP_LED_B[] PROGMEM = "led <#rrggbb> [<brightness>]";
const char CLI_HELP_LED_ENABLE[] PROGMEM = "led <enable/disable>";
const char CLI_HELP_DRAW[] PROGMEM = "draw";
const char CLI_HELP_SCREEN_ON[] PROGMEM = "screen <on/off>";
const char CLI_HELP_SCREEN_MODE[] PROGMEM = "screen mode <menu/packetmonitor/buttontest/loading>";

const char CLI_INPUT_PREFIX[] PROGMEM = "# ";
const char CLI_SERIAL_ENABLED[] PROGMEM = "Serial interface enabled";
const char CLI_SERIAL_DISABLED[] PROGMEM = "Serial interface disabled";
const char CLI_ERROR[] PROGMEM = "ERROR: ";
const char CLI_ERROR_PARAMETER[] PROGMEM = "Error Invalid parameter \"";
const char CLI_STOPPED_SCRIPT[] PROGMEM = "Cleared CLI command queue";
const char CLI_CONTINUOUSLY[] PROGMEM = "continuously";
const char CLI_EXECUTING[] PROGMEM = "Executing ";
const char CLI_SCRIPT_DONE_CONTINUE[] PROGMEM = "Done executing script - type 'stop script' to end the continuous mode";
const char CLI_SCRIPT_DONE[] PROGMEM = "Done executing script";
const char CLI_HELP_HEADER[] PROGMEM = "[===== List of commands =====]";
const char CLI_HELP_FOOTER[] PROGMEM = "========================================================================\r\nfor more information please visit github.com/spacehuhn/esp8266_deauther\r\n========================================================================";
const char CLI_ERROR_NAME_LEN[] PROGMEM = "ERROR : Name length 0";
const char CLI_ERROR_MAC_LEN[] PROGMEM = "ERROR : MAC length 0";
const char CLI_RICE_START[] PROGMEM = "Starting rice debugger (auto - repair mode enabled), please stand by...\r\nKeep the device connected to a power supply until the debugger ends\r\nYOU RISK TO BRICK THE BOARD!!!";
const char CLI_RICE_OUTPUT[] PROGMEM = "[ % d % % ]\r\n";
const char CLI_RICE_ERROR[] PROGMEM = "ERROR : Memory check failure at block 0x";
const char CLI_RICE_MEM[] PROGMEM = "Checking memory block 0x";
const char CLI_CHICKEN_OUTPUT[] PROGMEM = "                                                                 ` - : /////////:-.                    \r\n"
                                          "                                                            ./++so:`   ``    `.:/++/.               \r\n"
                                          "                                                        `/+o+.+o:.s:-++//s`        `:++-            \r\n"
                                          "                                                     `/+oo+//d-   oh/    s-            :o/`         \r\n"
                                          "                                                   .++.o+` `h-   .d.     h`://+`         .o+        \r\n"
                                          "                                                 .o+` +/   +o    y-     +d+.  .y           .s-      \r\n"
                                          "                                                +o`   h    d`   `/     .h-    `h             ++     \r\n"
                                          "                                              .s-     d    -           .`     +/              /o    \r\n"
                                          "                                             :o`      y.                     -y                /+   \r\n"
                                          "                                            /+        :o       `            -h`                 s:  \r\n"
                                          "                                           /o          y..://///////:`     /o/o.                `h  \r\n"
                                          "                                          -s           +o:``       `-++::/+-  `o/                o: \r\n"
                                          "                                          y.          :o              `:::.`   `oo`              -s \r\n"
                                          "                                         -s           h`            .++:---/+/+/:::++.           `h \r\n"
                                          "                                         +/           h            :o`   `  `/s  `  .s-           d \r\n"
                                          "                                         o:          .s            h`   /h-   o:/h-  -s          `h \r\n"
                                          "                                         +/          +/            h`   ``    s- `   +/          -s \r\n"
                                          "                                         .y         `h`            -s-      `+y-.`.:+/           +: \r\n"
                                          "                                          o:        o:              `/+/:/+ss:.-:y/.`           `h` \r\n"
                                          "   .:-`                                   `y-      ++                  `so::-://+y.             +/  \r\n"
                                          "  :o.-/+:  :+//:`                          `s:   `+/                   -h//:::---:/o-          -y   \r\n"
                                          "  :o   `:o:h. `-+/`                         -d+.:o-                    .y.``...-/y/++`        `y.   \r\n"
                                          "   +/    `:hs    -o-                        o/:/yo:-`                   +y++s//+/.           `s.    \r\n"
                                          "    /o`    `oo`   `/+`                   .-:y/-`+:+so+/:-`              s-  y:              -s.     \r\n"
                                          " ++//+y:     -+     .o:            ``-:/+:-.`.:+/:hs+`++:/o/:.`        `h   .y`            /o`      \r\n"
                                          "`h` `./ys-            :o-   .--:////:-.`        `-/o/::.`/sh-:os/:.`   .y oo`+/          -o:        \r\n"
                                          " :o-   `-o+.           `/+o/:-..`                   `.:+++o/``/:-oo++/:.so+://`       `:+/`         \r\n"
                                          "  `/+:`   ..             `++`                           `.-/+/:-/sy.`+o:+y/-.      .-/+-`           \r\n"
                                          "    `-+/-                  .-                                `.:/o+:-:.```-:oy/:://:-`              \r\n"
                                          "       .:+/.                                                      `.-:/+/::s/-..`                   \r\n"
                                          "          .++.                                                          `.-h.                       \r\n"
                                          "            .o/                                                            +/                       \r\n"
                                          "              :o.                                                          :o                       \r\n"
                                          "               .o:                                                         -s                       \r\n"
                                          "                 /o`                                                       :+                       \r\n"
                                          "                  -o-                                                      o:                       \r\n"
                                          "                   `o/                                                     h`                       \r\n"
                                          "                     :o.                                                  -s                        \r\n"
                                          "                      .o:                                                 y.                        \r\n"
                                          "                        /o.                                              /+                         \r\n"
                                          "                         .+/`                                           -s                          \r\n"
                                          "                           -+/.                                        .s`                          \r\n"
                                          "                             ./+/.`                                   -s`                           \r\n"
                                          "                                .:/+:.`                              /o`                            \r\n"
                                          "                                    .:/o/.`                        .o:                              \r\n"
                                          "                                       o/:/+/.`                  .++`                               \r\n"
                                          "                                       -s   `:/+:`            `:+/`                                 \r\n"
                                          "                                        ++`     -+o-`      `-++-                                    \r\n"
                                          "                                         :s/::/+//::+/---/+/:`                                      \r\n"
                                          "                                          +/s:`      `-h-s-                                         \r\n"
                                          "                                          +/s-        `y y.                                         \r\n"
                                          "                                          +/y.        `y h`                                         \r\n"
                                          "                                          //s:`       `y d                                          \r\n"
                                          "                                          +/-:/++/-`  `y h-`                                        \r\n"
                                          "                                          y:hs-ysosss..y --/+++/-`                                  \r\n"
                                          "                                          ds:`s:o+`-:`o:oos./h++osoo`                               \r\n"
                                          "                                          ::   o+++   h:y `o+.s:`.::                                \r\n"
                                          "                                                -+-   -/`   :s.++                                   \r\n"
                                          "                                                             `/+-   ";
const char CLI_SYSTEM_INFO[] PROGMEM = "[======== SYSTEM INFO ========]";
const char CLI_SYSTEM_OUTPUT[] PROGMEM = "RAM usage: %u bytes used [%d%%], %u bytes free [%d%%], %u bytes in total\r\n";
const char CLI_SYSTEM_AP_MAC[] PROGMEM = "AP MAC address: ";
const char CLI_SYSTEM_ST_MAC[] PROGMEM = "Station MAC address: ";
const char CLI_SYSTEM_RAM_OUT[] PROGMEM = "SPIFFS: %u bytes used [%d%%], %u bytes free [%d%%], %u bytes in total\r\n";
const char CLI_SYSTEM_SPIFFS_OUT[] PROGMEM = "        block size %u bytes, page size %u bytes\r\n";
const char CLI_FILES[] PROGMEM = "Files: ";
const char CLI_BYTES[] PROGMEM = " bytes";
const char CLI_SYSTEM_FOOTER[] PROGMEM = "===============================";
const char CLI_FORMATTING_SPIFFS[] PROGMEM = "Formatting SPIFFS...";
const char CLI_REMOVED[] PROGMEM = "Removed ";
const char CLI_ERROR_REMOVING[] PROGMEM = "ERROR: removing ";
const char CLI_REMOVING_LINES[] PROGMEM = "Removed lines ";
const char CLI_COPIED_FILES[] PROGMEM = "Copied file";
const char CLI_ERROR_COPYING[] PROGMEM = "ERROR: Copying file";
const char CLI_RENAMED_FILE[] PROGMEM = "Renamed file";
const char CLI_ERROR_RENAMING_FILE[] PROGMEM = "ERROR: Renaming file";
const char CLI_WRITTEN[] PROGMEM = "Written \"";
const char CLI_TO[] PROGMEM = "\" to ";
const char CLI_REPLACED_LINE[] PROGMEM = "Replaced line ";
const char CLI_WITH[] PROGMEM = " with ";
const char CLI_ERROR_REPLACING_LINE[] PROGMEM = "ERROR: replacing line in ";
const char CLI_INFO_HEADER[] PROGMEM = "====================================================================================";
const char CLI_INFO_SOFTWARE[] PROGMEM = "ESP8266 Deauther ";
const char CLI_INFO_COPYRIGHT[] PROGMEM = "2018 (c) Stefan Kremser";
const char CLI_INFO_LICENSE[] PROGMEM = "This software is licensed under the MIT License.";
const char CLI_INFO_ADDON[] PROGMEM = "For more information please visit github.com/spacehuhn/esp8266_deauther";
const char CLI_DEAUTHING[] PROGMEM = "Deauthing ";
const char CLI_ARROW[] PROGMEM = " -> ";
const char CLI_SENDING_BEACON[] PROGMEM = "Sending Beacon \"";
const char CLI_SENDING_PROBE[] PROGMEM = "Sending Probe \"";
const char CLI_CUSTOM_SENT[] PROGMEM = "Sent out custom packet";
const char CLI_CUSTOM_FAILED[] PROGMEM = "Sending custom packet failed";
const char CLI_DRAW_OUTPUT[] PROGMEM = "%+4u";
const char CLI_ERROR_NOT_FOUND_A[] PROGMEM = "ERROR: command \"";
const char CLI_ERROR_NOT_FOUND_B[] PROGMEM = "\" not found :(";
const char CLI_SYSTEM_CHANNEL[] PROGMEM = "Current WiFi channel: ";
const char CLI_CHANGED_SCREEN[] PROGMEM = "Changed screen mode";
const char CLI_DEFAULT_AUTOSTART[] PROGMEM = "scan -t 5s\nsysinfo\n";
const char CLI_RESUMED[] PROGMEM = "Command Line resumed";

// ===== DISPLAY ===== //

// DEBUG MESSAGES
const char D_ERROR_NOT_ENABLED[] PROGMEM = "ERROR: Display not enabled";
const char D_MSG_DISPLAY_OFF[] PROGMEM = "Turned display off";
const char D_MSG_DISPLAY_ON[] PROGMEM = "Turned display on";

// LOADING SCREEN
const char DSP_SCAN_FOR[] PROGMEM = "Scan for";
const char DSP_APS[] PROGMEM = "APs";
const char DSP_STS[] PROGMEM = "STs";
const char DSP_PKTS[] PROGMEM = "Pkts";
const char DSP_S[] PROGMEM = "/s";
const char DSP_SCAN_DONE[] PROGMEM = "Done";

// ALL MENUS
const char D_BACK[] PROGMEM = "[BACK]";
const char D_REMOVE_ALL[] PROGMEM = "REMOVE ALL";
const char D_SELECT[] PROGMEM = "SELECT";
const char D_DESELECT[] PROGMEM = "DESELECT";
const char D_REMOVE[] PROGMEM = "REMOVE";
const char D_SELECT_ALL[] PROGMEM = "SELECT ALL";
const char D_DESELECT_ALL[] PROGMEM = "DESELECT ALL";
const char D_CLONE[] PROGMEM = "CLONE SSID";
const char D_LED[] PROGMEM = "LED";

// BUTTON TEST
const char D_UP[] PROGMEM = "UP:";
const char D_DOWN[] PROGMEM = "DOWN:";
const char D_LEFT[] PROGMEM = "LEFT:";
const char D_RIGHT[] PROGMEM = "RIGHT:";
const char D_B[] PROGMEM = "A:";
const char D_A[] PROGMEM = "B:";

// MAIN MENU
const char D_SCAN[] PROGMEM = "SCAN";
const char D_SHOW[] PROGMEM = "SELECT";
const char D_ATTACK[] PROGMEM = "ATTACK";
const char D_PACKET_MONITOR[] PROGMEM = "PACKET MONITOR";
const char D_CLOCK[] PROGMEM = "CLOCK";

// SCAN MENU
const char D_SCAN_APST[] PROGMEM = "SCAN AP + ST";
const char D_SCAN_AP[] PROGMEM = "SCAN APs";
const char D_SCAN_ST[] PROGMEM = "SCAN Stations";

// SHOW MENU
const char D_ACCESSPOINTS[] PROGMEM = "APs ";
const char D_STATIONS[] PROGMEM = "Stations ";
const char D_NAMES[] PROGMEM = "Names ";
const char D_SSIDS[] PROGMEM = "SSIDs ";

// SSID LIST MENU
const char D_CLONE_APS[] PROGMEM = "CLONE APs";
const char D_RANDOM_MODE[] PROGMEM = "RANDOM MODE";

// ATTACK MENU
const char D_DEAUTH[] PROGMEM = "DEAUTH";
const char D_BEACON[] PROGMEM = "BEACON";
const char D_PROBE[] PROGMEM = "PROBE";
const char D_START_ATTACK[] PROGMEM = "START";
const char D_STOP_ATTACK[] PROGMEM = "STOP";

// SUB MENUS
const char D_ENCRYPTION[] PROGMEM = "Encryption:";
const char D_RSSI[] PROGMEM = "RSSI:";
const char D_CHANNEL[] PROGMEM = "Channel:";
const char D_CH[] PROGMEM = "Ch";
const char D_VENDOR[] PROGMEM = "Vendor:";
const char D_AP[] PROGMEM = "AP:";
const char D_PKTS[] PROGMEM = "pkts";
const char D_SEEN[] PROGMEM = "Seen:";

// ===== STATIONS ===== //
const char ST_CLEARED_LIST[] PROGMEM = "Cleared station list";
const char ST_REMOVED_STATION[] PROGMEM = "Removed station ";
const char ST_LIST_EMPTY[] PROGMEM = "Station list is empty :(";
const char ST_HEADER[] PROGMEM = "[===== Stations =====]";
const char ST_NO_DEVICES_SELECTED[] PROGMEM = "No devices selected";
const char ST_TABLE_HEADER[] PROGMEM = "ID MAC               Ch Name             Vendor   Pkts     AP                               Last Seen Selected";
const char ST_TABLE_DIVIDER[] PROGMEM = "==============================================================================================================";
const char ST_SMALLER_ONESEC[] PROGMEM = "<1sec";
const char ST_SMALLER_ONEMIN[] PROGMEM = "<1min";
const char ST_BIGER_ONEHOUR[] PROGMEM = ">1h";
const char ST_SELECTED_STATION[] PROGMEM = "Selected station ";
const char ST_DESELECTED_STATION[] PROGMEM = "Deselected station ";
const char ST_ERROR_ID[] PROGMEM = "ERROR: No station found with ID ";
const char ST_SELECTED_ALL[] PROGMEM = "Selected all stations";
const char ST_DESELECTED_ALL[] PROGMEM  = "Deselected all stations";

// ===== ACCESS POINTS ===== //
const char AP_HEADER[] PROGMEM = "[===== Access Points =====]";
const char AP_LIST_EMPTY[] PROGMEM = "AP list is empty :(";
const char AP_NO_AP_SELECTED[] PROGMEM = "No APs selected";
const char AP_TABLE_HEADER[] PROGMEM = "ID SSID                             Name             Ch RSSI Enc. Mac               Vendor   Selected";
const char AP_TABLE_DIVIDER[] PROGMEM = "=====================================================================================================";
const char AP_HIDDE_SSID[] PROGMEM = "*HIDDEN*";
const char AP_WEP[] PROGMEM = "WEP";
const char AP_WPA[] PROGMEM = "WPA";
const char AP_WPA2[] PROGMEM = "WPA2";
const char AP_AUTO[] PROGMEM = "WPA*";
const char AP_SELECTED[] PROGMEM = "Selected access point ";
const char AP_DESELECTED[] PROGMEM = "Deselected access point ";
const char AP_REMOVED[] PROGMEM = "Removed access point ";
const char AP_SELECTED_ALL[] PROGMEM = "Selected all APs";
const char AP_DESELECTED_ALL[] PROGMEM = "Deselected all APs";
const char AP_REMOVED_ALL[] PROGMEM = "Removed all APs";
const char AP_NO_AP_ERROR[] PROGMEM = "ERROR: No AP found with ID ";

// ===== ATTACKS ===== //
const char A_START[] PROGMEM = "Start attacking";
const char A_NO_MODE_ERROR[] PROGMEM = "WARNING: No valid attack mode set";
const char A_STOP[] PROGMEM = "Stopped attacking";
const char A_TIMEOUT[] PROGMEM = "Timeout - ";
const char A_STATUS[] PROGMEM = "[Pkt/s] All: %+4u | Deauths: %+3u/%-3u | Beacons: %+3u/%-3u | Probes: %+3u/%-3u\r\n";
const char A_ENABLED_OUTPUT[] PROGMEM = "Enabled attack output";
const char A_DISABLED_OUTPUT[] PROGMEM = "Disabled attack output";

// ===== NAMES ===== //
const char N_SAVED[] PROGMEM = "Device names saved in ";
const char N_LOADED[] PROGMEM = "Device names loaded from ";
const char N_REMOVED_ALL[] PROGMEM = "Removed all saved device names";
const char N_HEADER[] PROGMEM = "[===== Saved Devices =====]";
const char N_TABLE_HEADER[] PROGMEM =  "ID MAC               Vendor   Name             AP-BSSID          Ch Selected";
const char N_TABLE_DIVIDER[] PROGMEM = "============================================================================";
const char N_ERROR_LIST_EMPTY[] PROGMEM = "Device name list is empty :(";
const char N_ERROR_NO_SELECTED[] PROGMEM = "No devices selected";
const char N_ERROR_LIST_FULL[] PROGMEM = "ERROR: Name list is full!";
const char N_ADDED[] PROGMEM = "Added to device name list ";
const char N_REPLACED[] PROGMEM = "Replaced device name list ";
const char N_REMOVED[] PROGMEM = "Removed from the device name list ";
const char N_CHANGED_NAME[] PROGMEM = "Changed device name";
const char N_CHANGED_MAC[] PROGMEM = "Changed device mac";
const char N_CHANGED_BSSID[] PROGMEM = "Changed device AP-BSSID";
const char N_CHANGED_CH[] PROGMEM = "Changed device channel";
const char N_SELECTED[] PROGMEM = "Selected device ";
const char N_ERROR_NOT_FOUND[] PROGMEM = "No device found with name ";
const char N_DESELECTED[] PROGMEM = "Deselected device ";
const char N_SELECTED_ALL[] PROGMEM = "Selected all device names";
const char N_DESELECTED_ALL[] PROGMEM = "Deselected all device names";

// ===== SSIDs ===== //
const char SS_LOADED[] PROGMEM = "SSIDs loaded from ";
const char SS_CLEARED[] PROGMEM = "Cleared SSID list";
const char SS_SAVED[] PROGMEM = "SSIDs saved";
const char SS_SAVED_IN[] PROGMEM = "SSIDs saved in ";
const char SS_REMOVED[] PROGMEM = "Removed SSID ";
const char SS_ERROR_FULL[] PROGMEM =
    "ERROR: SSID list is full! Remove some SSIDs first or run command with -f (force) parameter.";
const char SS_ADDED[] PROGMEM = "Added SSID ";
const char SS_REPLACED[] PROGMEM = "Replaced SSID ";
const char SS_TABLE_HEADER[] PROGMEM = "ID Enc. SSID";
const char SS_TABLE_DIVIDER[] PROGMEM = "=========================================";
const char SS_HEADER[] PROGMEM = "[===== SSIDs =====]";
const char SS_ERROR_EMPTY[] PROGMEM = "SSID list is empty :(";
const char SS_RANDOM_ENABLED[] PROGMEM = "SSID random mode enabled";
const char SS_RANDOM_DISABLED[] PROGMEM = "SSID random mode deactivated";
const char SS_JSON_SSIDS[] PROGMEM = "ssids";
const char SS_JSON_RANDOM[] PROGMEM = "random";
const char SS_JSON_DEFAULT[] PROGMEM = "{\"random\":false,\"ssids\":[[\"Never gonna give you up\",false,23],[\"Never gonna let you down\",false,24],[\"Never gonna run around\",false,22],[\"Never gonna make you cry\",false,24],[\"Never gonna say goodbye\",false,23],[\"Never gonna tell a lie\",false,22],[\"Never gonna hurt you\",false,20],[\"Never gonna desert you\",false,22]]}";
const char SS_RANDOM_INFO[] PROGMEM = "Generating new SSIDs... Type \"disable random\" to stop the random mode";

// ===== SCAN ==== //
const char SC_START_CLIENT[] PROGMEM = "Starting Scan for stations (client devices) - ";
const char SS_START_SNIFFER[] PROGMEM = "Starting packet sniffer - ";
const char SC_ERROR_NO_AP[] PROGMEM =
    "ERROR: AP-list empty! Can't scan for clients, please Scan for Accesspoints first.";
const char SC_INFINITELY[] PROGMEM = " infinitely";
const char SC_ON_CHANNEL[] PROGMEM = " on channel ";
const char SC_START_AP[] PROGMEM = "Starting scan for access points (Wi-Fi networks)...";
const char SC_ONE_TO[] PROGMEM = "1 - ";
const char SC_STOPPED[] PROGMEM = "Stopped scan";
const char SC_RESTRAT[] PROGMEM = "Scan will restart in ";
const char SC_CONTINUE[] PROGMEM = "s - type stop to disable the continuous mode";
const char SC_RESTART[] PROGMEM = "restarting in ";
const char SC_ERROR_MODE[] PROGMEM = "ERROR: Invalid scan mode ";
const char SC_OUTPUT_A[] PROGMEM = "Scanning WiFi [%+2u%%]: %+3u packets/s | %+2u devices | %+2u deauths\r\n";
const char SC_OUTPUT_B[] PROGMEM = "Scanning WiFi: %+3u packets/s | %+2u devices | %+2u deauths\r\n";
const char SC_JSON_APS[] PROGMEM = "aps";
const char SC_JSON_STATIONS[] PROGMEM = "stations";
const char SC_JSON_NAMES[] PROGMEM = "names";
const char SC_SAVED[] PROGMEM = "Saved scan results";
const char SC_SAVED_IN[] PROGMEM = "Scan results saved in ";
const char SC_MODE_OFF[] PROGMEM = "-";
const char SC_MODE_AP[] PROGMEM = "APs";
const char SC_MODE_ST[] PROGMEM = "STs";
const char SC_MODE_ALL[] PROGMEM = "AP+ST";
const char SC_MODE_SNIFFER[] PROGMEM = "Sniffer";

// ===== FUNCTIONS ===== //
const char F_ERROR_MAC[] PROGMEM = "ERROR: MAC address invalid";
const char F_COLOR_INVALID[] PROGMEM = "ERROR: Color code invalid";
const char F_ERROR_READING_FILE[] PROGMEM = "ERROR: reading file ";
const char F_LINE[] PROGMEM = "[%d] ";
const char F_ERROR_FILE[] PROGMEM = "ERROR: File doesn't exist ";
const char F_ERROR_OPEN[] PROGMEM = "ERROR couldn't open ";
const char F_ERROR_PARSING_JSON[] PROGMEM = "ERROR parsing JSON ";
const char F_ERROR_TO_BIG[] PROGMEM = "ERROR file too big ";
const char F_TMP[] PROGMEM = "/tmp";
const char F_COPY[] PROGMEM = "_copy";
const char F_ERROR_SAVING[] PROGMEM = "ERROR: saving file. Try 'format' and restart - ";

// ===== WIFI ===== //
const char W_STOPPED_AP[] PROGMEM = "Stopped Access Point";
const char W_AP_REQUEST[] PROGMEM = "[AP] request: ";
const char W_AP[] PROGMEM = "AP";
const char W_STATION[] PROGMEM = "Station";
const char W_MODE_OFF[] PROGMEM = "OFF";
const char W_MODE_AP[] PROGMEM = "AP";
const char W_MODE_ST[] PROGMEM = "STATION";
const char W_OK[] PROGMEM = " OK";
const char W_NOT_FOUND[] PROGMEM = " NOT FOUND";
const char W_BAD_ARGS[] PROGMEM = "BAD ARGS";
const char W_BAD_PATH[] PROGMEM = "BAD PATH";
const char W_FILE_NOT_FOUND[] PROGMEM = "ERROR 404 File Not Found";
const char W_STARTED_AP[] PROGMEM = "Started AP";

#endif // ifndef language_h