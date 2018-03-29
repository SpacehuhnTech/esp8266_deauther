#ifndef language_h
#define language_h

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
static const char CURSOR = '|';
static const char SPACE = ' ';
static const char DOUBLEPOINT = ':';
static const char EQUALS = '=';
static const char HASHSIGN = '#';
static const char ASTERIX = '*';
static const char PERCENT = '%';
static const char DASH = '-';
static const char QUESTIONMARK = '?';
static const char ZERO = '0';
static const char S = 's';
static const char M = 'm';
static const char D = 'd';
static const char DOUBLEQUOTES = '\"';
static const char SLASH = '/';
static const char NEWLINE = '\n';
static const char CARRIAGERETURN = '\r';
static const char SEMICOLON = ';';
static const char BACKSLASH = '\\';
static const char POINT = '.';
static const char VERTICALBAR = '|';
static const char COMMA = ',';
static const char ENDOFLINE = '\0';
static const char OPEN_BRACKET = '[';
static const char CLOSE_BRACKET = ']';
static const char OPEN_CURLY_BRACKET = '{';
static const char CLOSE_CURLY_BRACKET = '}';

static const char STR_TRUE[] PROGMEM = "true";
static const char STR_FALSE[] PROGMEM = "false";
static const char STR_MIN[] PROGMEM = "min";

// ===== GLOBAL FUNCTIONS ===== //

// for reading Strings from the PROGMEM
static String str(const char* ptr){
  char keyword[strlen_P(ptr)];
  strcpy_P(keyword, ptr);
  return String(keyword);
}

// for converting keywords
static String keyword(const char* keywordPtr){
  char keyword[strlen_P(keywordPtr)];
  strcpy_P(keyword, keywordPtr);
  
  String str = "";
  uint8_t len = strlen(keyword);
  uint8_t i = 0;
  
  while(i<len && keyword[i] != SLASH && keyword[i] != COMMA){
    str += keyword[i];
    i++;
  }
  
  return str;
}

static const char S_DEAUTHSPERTARGET[] PROGMEM = "deauthspertarget";

// equals function
static bool eqls(const char* str, const char* keywordPtr){
  if(strlen(str) > 255) return false; // when string too long

  char keyword[strlen_P(keywordPtr) + 1];
  strcpy_P(keyword, keywordPtr);
  
  uint8_t lenStr = strlen(str);
  uint8_t lenKeyword = strlen(keyword);
  if(lenStr > lenKeyword) return false; // string can't be longer than keyword (but can be smaller because of '/' and ',')
  
  uint8_t a = 0;
  uint8_t b = 0;
  bool result = true;
  while(a < lenStr && b < lenKeyword){
    if(keyword[b] == SLASH || keyword[b] == COMMA) b++;
    if(tolower(str[a]) != tolower(keyword[b])) result = false;
    if((a == lenStr && !result) || !result){ // fast forward to next comma
      while(b < lenKeyword && keyword[b] != COMMA) b++;
      result = true;
      a = 0;
    } else {
      a++;
      b++;
    }
  }
  // comparison correct AND string checked until the end AND keyword checked until the end
  return result && a == lenStr && (keyword[b] == COMMA || keyword[b] == SLASH || keyword[b] == ENDOFLINE);
}

static bool eqls(String str, const char* keywordPtr){
  return eqls(str.c_str(), keywordPtr);
}

// boolean to string
static String b2s(bool input){
  return str(input ? STR_TRUE : STR_FALSE);
}

// boolean to asterix *
static String b2a(bool input){
  return (input ? String(ASTERIX) : String(SPACE));
}

// string to boolean
static bool s2b(String input){
  return eqls(input, STR_TRUE);
}

// ===== PRINT FUNCTIONS ===== //
static void prnt(String s){
  Serial.print(s);
}
static void prnt(bool b){
  Serial.print(b2s(b));
}
static void prnt(char c){
  Serial.print(c);
}
static void prnt(const char* ptr){
  Serial.print(FPSTR(ptr));
}
static void prnt(int i){
  Serial.print((String)i);
}

static void prntln(){
  Serial.println();
}
static void prntln(String s){
  Serial.println(s);
}
static void prntln(bool b){
  Serial.println(b2s(b));
}
static void prntln(char c){
  Serial.println(c);
}
static void prntln(const char* ptr){
  Serial.println(FPSTR(ptr));
}
static void prntln(int i){
  Serial.println((String)i);
}

// ===== SETUP ===== //
static const char SETUP_OK[] PROGMEM = "OK";
static const char SETUP_ERROR[] PROGMEM = "ERROR";
static const char SETUP_MOUNT_SPIFFS[] PROGMEM = "Mounting SPIFFS...";
static const char SETUP_FORMAT_SPIFFS[] PROGMEM = "Formatting SPIFFS...";
static const char SETUP_SERIAL_WARNING[] PROGMEM = "Warning: Serial deactivated";
static const char SETUP_STARTED[] PROGMEM = "STARTED! \\o/";
static const char SETUP_COPYING[] PROGMEM = "Copying ";
static const char SETUP_PROGMEM_TO_SPIFFS[] PROGMEM = " from PROGMEM to SPIFFS...";

// ===== SERIAL COMMAND LINE INTERFACE ===== //
static const char CLI_SCAN[] PROGMEM = "scan";                          // scan
static const char CLI_REBOOT[] PROGMEM = "reboot";                      // reboot
static const char CLI_STATUS[] PROGMEM = "status";                      // status
static const char CLI_SHOW[] PROGMEM = "show";                          // show
static const char CLI_REMOVE[] PROGMEM = "remove";                      // remove
static const char CLI_SET[] PROGMEM = "set";                            // set
static const char CLI_STOP[] PROGMEM = "stop";                          // stop
static const char CLI_LOAD[] PROGMEM = "load";                          // load
static const char CLI_SAVE[] PROGMEM = "save";                          // save
static const char CLI_ADD[] PROGMEM = "add";                            // add
static const char CLI_DESELECT[] PROGMEM = "deselect";                  // deselect
static const char CLI_CLEAR[] PROGMEM = "clear";                        // clear
static const char CLI_SYSINFO[] PROGMEM = "sysinfo";                    // sysinfo
static const char CLI_RESET[] PROGMEM = "reset";                        // reset
static const char CLI_ON[] PROGMEM = "on";                              // on
static const char CLI_OFF[] PROGMEM = "off";                            // off
static const char CLI_RANDOM[] PROGMEM = "random";                      // random
static const char CLI_GET[] PROGMEM = "get";                            // get
static const char CLI_INFO[] PROGMEM = "info";                          // info
static const char CLI_HELP[] PROGMEM = "help";                          // help
static const char CLI_RICE[] PROGMEM = "rice";                          // rice
static const char CLI_FORMAT[] PROGMEM = "format";                      // format
static const char CLI_DELETE[] PROGMEM = "delete";                      // delete
static const char CLI_PRINT[] PROGMEM = "print";                        // print
static const char CLI_RUN[] PROGMEM = "run";                            // run
static const char CLI_WRITE[] PROGMEM = "write";                        // write
static const char CLI_LED[] PROGMEM = "led";                            // led
static const char CLI_SEND[] PROGMEM = "send";                          // send
static const char CLI_CUSTOM[] PROGMEM = "custom";                      // custom
static const char CLI_DELAY[] PROGMEM = "delay";                        // delay
static const char CLI_REPLACE[] PROGMEM = "replace";                    // replace
static const char CLI_DRAW[] PROGMEM = "draw";                          // draw
static const char CLI_SCRIPT[] PROGMEM = "script";                      // script
static const char CLI_STARTAP[] PROGMEM = "startap";                    // startap
static const char CLI_STOPAP[] PROGMEM = "stopap";                      // stopap
static const char CLI_RENAME[] PROGMEM = "rename";                      // rename
static const char CLI_COPY[] PROGMEM = "copy";                          // copy
static const char CLI_ENABLE[] PROGMEM = "enable/d";                    // enable, enabled
static const char CLI_DISABLE[] PROGMEM = "disable/d";                  // disable, disabled
static const char CLI_WPA2[] PROGMEM = "wpa/2,-wpa/2";                  // wpa, wpa2, -wpa, -wpa2
static const char CLI_ATTACK[] PROGMEM = "attack/s";                    // attack, attacks
static const char CLI_CHICKEN[] PROGMEM = "chicken/s";                  // chicken, chickens
static const char CLI_SETTING[] PROGMEM = "setting/s";                  // setting, settings
static const char CLI_ID[] PROGMEM = "id,-i/d";                         // id, -i, -id
static const char CLI_ALL[] PROGMEM = "all,-a";                         // all, -a
static const char CLI_TIME[] PROGMEM = "time,-t";                       // time, -t
static const char CLI_CONTINUE[] PROGMEM = "continue,-c";               // continue, -c
static const char CLI_CHANNEL[] PROGMEM = "channel,-ch";                // channel, -ch
static const char CLI_MAC[] PROGMEM = "mac,-m";                         // mac, -m
static const char CLI_BSSID[] PROGMEM = "bssid,-b";                     // bssid, -b
static const char CLI_BEACON[] PROGMEM = "beacon,-b";                   // bssid, -b
static const char CLI_DEAUTH[] PROGMEM = "deauth,-d";                   // deauth, -d
static const char CLI_DEAUTHALL[] PROGMEM = "deauthall,-da";            // deauthall, -da
static const char CLI_PROBE[] PROGMEM = "probe,-p";                     // probe, -p
static const char CLI_NOOUTPUT[] PROGMEM = "nooutput,-no";              // nooutput, -no
static const char CLI_FORCE[] PROGMEM = "force,-f";                     // force, -f
static const char CLI_TIMEOUT[] PROGMEM = "timeout,-t";                 // timeout, -t
static const char CLI_WIFI[] PROGMEM = "wifi,-w";                       // wifi, -w
static const char CLI_CLONES[] PROGMEM = "clones,-cl";                  // clones, -cl
static const char CLI_PATH[] PROGMEM = "path,-p";                       // path, -p
static const char CLI_PASSWORD[] PROGMEM = "password,-ps/wd";           // password, -ps, -pswd
static const char CLI_HIDDEN[] PROGMEM = "hidden,-h";                   // hidden, -h
static const char CLI_CAPTIVEPORTAL[] PROGMEM = "captiveportal,-cp";    // captiveportal, -cp
static const char CLI_SELECT[] PROGMEM = "select/ed,-s";                // select, selected, -s
static const char CLI_SSID[] PROGMEM = "ssid/s,-s/s";                   // ssid, ssids, -s, -ss
static const char CLI_AP[] PROGMEM = "ap/s,-ap/s";                      // ap, aps, -ap, -aps
static const char CLI_STATION[] PROGMEM = "station/s,-st";              // station, stations, -st
static const char CLI_NAME[] PROGMEM = "name/s,-n";                     // name, names, -n
static const char CLI_LINE[] PROGMEM = "line/s,-l";                     // line, lines, -l
static const char CLI_COMMENT[] PROGMEM = "//";                         // // 
static const char CLI_SCREEN[] PROGMEM = "screen";                      // screen
static const char CLI_MODE[] PROGMEM = "mode,-m";                       // mode
static const char CLI_MODE_BUTTONTEST[] PROGMEM = "buttontest";         // buttontest
static const char CLI_MODE_PACKETMONITOR[] PROGMEM = "packetmonitor";   // packetmonitor
static const char CLI_MODE_LOADINGSCREEN[] PROGMEM = "loadingscreen";   // loading
static const char CLI_MODE_MENU[] PROGMEM = "menu";                     // menu

static const char CLI_HELP_HELP[] PROGMEM = "help";
static const char CLI_HELP_SCAN[] PROGMEM = "scan [<all/aps/stations>] [-t <time>] [-c <continue-time>] [-ch <channel>]";
static const char CLI_HELP_SHOW[] PROGMEM = "show [selected] [<all/aps/stations/names/ssids>]";
static const char CLI_HELP_SELECT[] PROGMEM = "select [<all/aps/stations/names>] [<id>]";
static const char CLI_HELP_DESELECT[] PROGMEM = "deselect [<all/aps/stations/names>] [<id>]";
static const char CLI_HELP_SSID_A[] PROGMEM = "add ssid <ssid> [-wpa2] [-cl <clones>]";
static const char CLI_HELP_SSID_B[] PROGMEM = "add ssid -ap <id> [-cl <clones>] [-f]";
static const char CLI_HELP_SSID_C[] PROGMEM = "add ssid -s [-f]";
static const char CLI_HELP_NAME_A[] PROGMEM = "add name <name> [-ap <id>] [-s]";
static const char CLI_HELP_NAME_B[] PROGMEM = "add name <name> [-st <id>] [-s]";
static const char CLI_HELP_NAME_C[] PROGMEM = "add name <name> [-m <mac>] [-ch <channel>] [-b <bssid>] [-s]";
static const char CLI_HELP_SET_NAME[] PROGMEM = "set name <id> <newname>";
static const char CLI_HELP_ENABLE_RANDOM[] PROGMEM = "enable random <interval>";
static const char CLI_HELP_DISABLE_RANDOM[] PROGMEM = "disable random";
static const char CLI_HELP_LOAD[] PROGMEM = "load [<all/ssids/names/settings>] [<file>]";
static const char CLI_HELP_SAVE[] PROGMEM = "save [<all/ssids/names/settings>] [<file>]";
static const char CLI_HELP_REMOVE_A[] PROGMEM = "remove <ap/station/name/ssid> <id>";
static const char CLI_HELP_REMOVE_B[] PROGMEM = "remove <ap/station/names/ssids> [all]";
static const char CLI_HELP_ATTACK[] PROGMEM = "attack [beacon] [deauth] [deauthall] [probe] [nooutput] [-t <timeout>]";
static const char CLI_HELP_ATTACK_STATUS[] PROGMEM = "attack status [<on/off>]";
static const char CLI_HELP_STOP[] PROGMEM = "stop <all/scan/attack>";
static const char CLI_HELP_SYSINFO[] PROGMEM = "sysinfo";
static const char CLI_HELP_CLEAR[] PROGMEM = "clear";
static const char CLI_HELP_FORMAT[] PROGMEM = "format";
static const char CLI_HELP_PRINT[] PROGMEM = "print <file> [<lines>]";
static const char CLI_HELP_DELETE[] PROGMEM = "delete <file> [<lineFrom>] [<lineTo>]";
static const char CLI_HELP_REPLACE[] PROGMEM = "replace <file> <line> <new-content>";
static const char CLI_HELP_COPY[] PROGMEM = "copy <file> <newfile>";
static const char CLI_HELP_RENAME[] PROGMEM = "rename <file> <newfile>";
static const char CLI_HELP_RUN[] PROGMEM = "run <file>";
static const char CLI_HELP_WRITE[] PROGMEM = "write <file> <commands>";
static const char CLI_HELP_GET[] PROGMEM = "get <setting>";
static const char CLI_HELP_SET[] PROGMEM = "set <setting> <value>";
static const char CLI_HELP_RESET[] PROGMEM = "reset";
static const char CLI_HELP_CHICKEN[] PROGMEM = "chicken";
static const char CLI_HELP_REBOOT[] PROGMEM = "reboot";
static const char CLI_HELP_INFO[] PROGMEM = "info";
static const char CLI_HELP_COMMENT[] PROGMEM = "// <comments>";
static const char CLI_HELP_SEND_DEAUTH[] PROGMEM = "send deauth <apMac> <stMac> <rason> <channel>";
static const char CLI_HELP_SEND_BEACON[] PROGMEM = "send beacon <mac> <ssid> <ch> [wpa2]";
static const char CLI_HELP_SEND_PROBE[] PROGMEM = "send probe <mac> <ssid> <ch>";
static const char CLI_HELP_LED_A[] PROGMEM = "led <r> <g> <b> [<brightness>]";
static const char CLI_HELP_LED_B[] PROGMEM = "led <#rrggbb> [<brightness>]";
static const char CLI_HELP_LED_ENABLE[] PROGMEM = "led <enable/disable>";
static const char CLI_HELP_DRAW[] PROGMEM = "draw";
static const char CLI_HELP_SCREEN_ON[] PROGMEM = "screen <on/off>";
static const char CLI_HELP_SCREEN_MODE[] PROGMEM = "screen mode <menu/packetmonitor/buttontest/loading>";

static const char CLI_INPUT_PREFIX[] PROGMEM = "# ";
static const char CLI_SERIAL_ENABLED[] PROGMEM = "Serial interface enabled";
static const char CLI_SERIAL_DISABLED[] PROGMEM = "Serial interface disabled";
static const char CLI_ERROR[] PROGMEM = "ERROR: ";
static const char CLI_ERROR_PARAMETER[] PROGMEM = "Error Invalid parameter \"";
static const char CLI_STOPPED_SCRIPT[] PROGMEM = "Stopped executing script \"";
static const char CLI_CONTINUOUSLY[] PROGMEM = "continuously";
static const char CLI_EXECUTING[] PROGMEM = "Executing ";
static const char CLI_SCRIPT_DONE_CONTINUE[] PROGMEM = "Done executing script - type 'stop script' to end the continuous mode";
static const char CLI_SCRIPT_DONE[] PROGMEM = "Done executing script";
static const char CLI_HELP_HEADER[] PROGMEM = "[===== List of commands =====]";
static const char CLI_HELP_FOOTER[] PROGMEM = "========================================================================\r\nfor more information please visit github.com/spacehuhn/esp8266_deauther\r\n========================================================================";
static const char CLI_ERROR_NAME_LEN[] PROGMEM = "ERROR : Name length 0";
static const char CLI_ERROR_MAC_LEN[] PROGMEM = "ERROR : MAC length 0";
static const char CLI_RICE_START[] PROGMEM = "Starting rice debugger (auto - repair mode enabled), please stand by...\r\nKeep the device connected to a power supply until the debugger ends\r\nYOU RISK TO BRICK THE BOARD!!!";
static const char CLI_RICE_OUTPUT[] PROGMEM = "[ % d % % ]\r\n";
static const char CLI_RICE_ERROR[] PROGMEM = "ERROR : Memory check failure at block 0x";
static const char CLI_RICE_MEM[] PROGMEM = "Checking memory block 0x";
static const char CLI_CHICKEN_OUTPUT[] PROGMEM = "                                                                 ` - : /////////:-.                    \r\n                                                            ./++so:`   ``    `.:/++/.               \r\n                                                        `/+o+.+o:.s:-++//s`        `:++-            \r\n                                                     `/+oo+//d-   oh/    s-            :o/`         \r\n                                                   .++.o+` `h-   .d.     h`://+`         .o+        \r\n                                                 .o+` +/   +o    y-     +d+.  .y           .s-      \r\n                                                +o`   h    d`   `/     .h-    `h             ++     \r\n                                              .s-     d    -           .`     +/              /o    \r\n                                             :o`      y.                     -y                /+   \r\n                                            /+        :o       `            -h`                 s:  \r\n                                           /o          y..://///////:`     /o/o.                `h  \r\n                                          -s           +o:``       `-++::/+-  `o/                o: \r\n                                          y.          :o              `:::.`   `oo`              -s \r\n                                         -s           h`            .++:---/+/+/:::++.           `h \r\n                                         +/           h            :o`   `  `/s  `  .s-           d \r\n                                         o:          .s            h`   /h-   o:/h-  -s          `h \r\n                                         +/          +/            h`   ``    s- `   +/          -s \r\n                                         .y         `h`            -s-      `+y-.`.:+/           +: \r\n                                          o:        o:              `/+/:/+ss:.-:y/.`           `h` \r\n   .:-`                                   `y-      ++                  `so::-://+y.             +/  \r\n  :o.-/+:  :+//:`                          `s:   `+/                   -h//:::---:/o-          -y   \r\n  :o   `:o:h. `-+/`                         -d+.:o-                    .y.``...-/y/++`        `y.   \r\n   +/    `:hs    -o-                        o/:/yo:-`                   +y++s//+/.           `s.    \r\n    /o`    `oo`   `/+`                   .-:y/-`+:+so+/:-`              s-  y:              -s.     \r\n ++//+y:     -+     .o:            ``-:/+:-.`.:+/:hs+`++:/o/:.`        `h   .y`            /o`      \r\n`h` `./ys-            :o-   .--:////:-.`        `-/o/::.`/sh-:os/:.`   .y oo`+/          -o:        \r\n :o-   `-o+.           `/+o/:-..`                   `.:+++o/``/:-oo++/:.so+://`       `:+/`         \r\n  `/+:`   ..             `++`                           `.-/+/:-/sy.`+o:+y/-.      .-/+-`           \r\n    `-+/-                  .-                                `.:/o+:-:.```-:oy/:://:-`              \r\n       .:+/.                                                      `.-:/+/::s/-..`                   \r\n          .++.                                                          `.-h.                       \r\n            .o/                                                            +/                       \r\n              :o.                                                          :o                       \r\n               .o:                                                         -s                       \r\n                 /o`                                                       :+                       \r\n                  -o-                                                      o:                       \r\n                   `o/                                                     h`                       \r\n                     :o.                                                  -s                        \r\n                      .o:                                                 y.                        \r\n                        /o.                                              /+                         \r\n                         .+/`                                           -s                          \r\n                           -+/.                                        .s`                          \r\n                             ./+/.`                                   -s`                           \r\n                                .:/+:.`                              /o`                            \r\n                                    .:/o/.`                        .o:                              \r\n                                       o/:/+/.`                  .++`                               \r\n                                       -s   `:/+:`            `:+/`                                 \r\n                                        ++`     -+o-`      `-++-                                    \r\n                                         :s/::/+//::+/---/+/:`                                      \r\n                                          +/s:`      `-h-s-                                         \r\n                                          +/s-        `y y.                                         \r\n                                          +/y.        `y h`                                         \r\n                                          //s:`       `y d                                          \r\n                                          +/-:/++/-`  `y h-`                                        \r\n                                          y:hs-ysosss..y --/+++/-`                                  \r\n                                          ds:`s:o+`-:`o:oos./h++osoo`                               \r\n                                          ::   o+++   h:y `o+.s:`.::                                \r\n                                                -+-   -/`   :s.++                                   \r\n                                                             `/+-   ";
static const char CLI_SYSTEM_INFO[] PROGMEM = "[======== SYSTEM INFO ========]";
static const char CLI_SYSTEM_OUTPUT[] PROGMEM = "RAM usage: %u bytes used [%d%%], %u bytes free [%d%%], %u bytes in total\r\n";
static const char CLI_SYSTEM_AP_MAC[] PROGMEM = "AP MAC address: ";
static const char CLI_SYSTEM_ST_MAC[] PROGMEM = "Station MAC address: ";
static const char CLI_SYSTEM_RAM_OUT[] PROGMEM = "SPIFFS: %u bytes used [%d%%], %u bytes free [%d%%], %u bytes in total\r\n";
static const char CLI_SYSTEM_SPIFFS_OUT[] PROGMEM = "        block size %u bytes, page size %u bytes\r\n";
static const char CLI_FILES[] PROGMEM = "Files: ";
static const char CLI_BYTES[] PROGMEM = " bytes";
static const char CLI_SYSTEM_FOOTER[] PROGMEM = "===============================";
static const char CLI_FORMATTING_SPIFFS[] PROGMEM = "Formatting SPIFFS...";
static const char CLI_REMOVED[] PROGMEM = "Removed ";
static const char CLI_ERROR_REMOVING[] PROGMEM = "ERROR: removing ";
static const char CLI_REMOVING_LINES[] PROGMEM = "Removed lines ";
static const char CLI_COPIED_FILES[] PROGMEM = "Copied file";
static const char CLI_ERROR_COPYING[] PROGMEM = "ERROR: Copying file";
static const char CLI_RENAMED_FILE[] PROGMEM = "Renamed file";
static const char CLI_ERROR_RENAMING_FILE[] PROGMEM = "ERROR: Renaming file";
static const char CLI_WRITTEN[] PROGMEM = "Written \"";
static const char CLI_TO[] PROGMEM = "\" to ";
static const char CLI_REPLACED_LINE[] PROGMEM = "Replaced line ";
static const char CLI_WITH[] PROGMEM = " with ";
static const char CLI_ERROR_REPLACING_LINE[] PROGMEM = "ERROR: replacing line in ";
static const char CLI_INFO_HEADER[] PROGMEM = "====================================================================================";
static const char CLI_INFO_SOFTWARE[] PROGMEM = "ESP8266 Deauther ";
static const char CLI_INFO_COPYRIGHT[] PROGMEM = "2018 (c) Stefan Kremser";
static const char CLI_INFO_LICENSE[] PROGMEM = "This software is licensed under the MIT License.";
static const char CLI_INFO_ADDON[] PROGMEM = "For more information please visit github.com/spacehuhn/esp8266_deauther";
static const char CLI_DEAUTHING[] PROGMEM = "Deauthing ";
static const char CLI_ARROW[] PROGMEM = " -> ";
static const char CLI_SENDING_BEACON[] PROGMEM = "Sending Beacon \"";
static const char CLI_SENDING_PROBE[] PROGMEM = "Sending Probe \"";
static const char CLI_CUSTOM_SENT[] PROGMEM = "Sent out custom packet";
static const char CLI_CUSTOM_FAILED[] PROGMEM = "Sending custom packet failed";
static const char CLI_DRAW_OUTPUT[] PROGMEM = "%+4u";
static const char CLI_ERROR_NOT_FOUND_A[] PROGMEM = "ERROR: command \"";
static const char CLI_ERROR_NOT_FOUND_B[] PROGMEM = "\" not found :(";
static const char CLI_SYSTEM_CHANNEL[] PROGMEM = "Current WiFi channel: ";
static const char CLI_CHANGED_SCREEN[] PROGMEM = "Changed screen mode";

// ===== DISPLAY ===== //

// DEBUG MESSAGES
static const char D_ERROR_NOT_ENABLED[] PROGMEM = "ERROR: Display not enabled";
static const char D_MSG_DISPLAY_OFF[] PROGMEM = "Turned display off";
static const char D_MSG_DISPLAY_ON[] PROGMEM = "Turned display on";

// LOADING SCREEN
static const char D_LOADING_SCREEN_0[] PROGMEM = "Scanning...";
static const char D_LOADING_SCREEN_1[] PROGMEM = "APs: ";
static const char D_LOADING_SCREEN_2[] PROGMEM = "Stations: ";
static const char D_LOADING_SCREEN_3[] PROGMEM = "Searching for";
static const char D_LOADING_SCREEN_4[] PROGMEM = "Access Points...";

// ALL MENUS
static const char D_BACK[] PROGMEM = "[BACK]";
static const char D_REMOVE_ALL[] PROGMEM = "REMOVE ALL";
static const char D_SELECT[] PROGMEM = "SELECT";
static const char D_DESELECT[] PROGMEM = "DESELECT";
static const char D_REMOVE[] PROGMEM = "REMOVE";
static const char D_SELECT_ALL[] PROGMEM = "SELECT ALL";
static const char D_DESELECT_ALL[] PROGMEM = "DESELECT ALL";
static const char D_CLONE[] PROGMEM = "CLONE SSID";

// BUTTON TEST
static const char D_UP[] PROGMEM = "UP:";
static const char D_DOWN[] PROGMEM = "DOWN:";
static const char D_LEFT[] PROGMEM = "LEFT:";
static const char D_RIGHT[] PROGMEM = "RIGHT:";
static const char D_B[] PROGMEM = "A:";
static const char D_A[] PROGMEM = "B:";

// MAIN MENU
static const char D_SCAN[] PROGMEM = "SCAN";
static const char D_SHOW[] PROGMEM = "SELECT";
static const char D_ATTACK[] PROGMEM = "ATTACK";
static const char D_PACKET_MONITOR[] PROGMEM = "PACKET MONITOR";

// SCAN MENU
static const char D_SCAN_APST[] PROGMEM = "SCAN AP + ST";
static const char D_SCAN_AP[] PROGMEM = "SCAN APs";
static const char D_SCAN_ST[] PROGMEM = "SCAN Stations";

// SHOW MENU
static const char D_ACCESSPOINTS[] PROGMEM = "APs ";
static const char D_STATIONS[] PROGMEM = "Stations ";
static const char D_NAMES[] PROGMEM = "Names ";
static const char D_SSIDS[] PROGMEM = "SSIDs ";

// SSID LIST MENU
static const char D_CLONE_APS[] PROGMEM = "CLONE APs";
static const char D_RANDOM_MODE[] PROGMEM = "RANDOM MODE";

// ATTACK MENU
static const char D_DEAUTH[] PROGMEM = "DEAUTH";
static const char D_BEACON[] PROGMEM = "BEACON";
static const char D_PROBE[] PROGMEM = "PROBE";
static const char D_START_ATTACK[] PROGMEM = "START";
static const char D_STOP_ATTACK[] PROGMEM = "STOP";

// SUB MENUS
static const char D_ENCRYPTION[] PROGMEM = "Encryption:";
static const char D_RSSI[] PROGMEM = "RSSI:";
static const char D_CHANNEL[] PROGMEM = "Channel:";
static const char D_CH[] PROGMEM = "CH:";
static const char D_VENDOR[] PROGMEM = "Vendor:";
static const char D_AP[] PROGMEM = "AP:";
static const char D_PKTS[] PROGMEM = "Pkts:";
static const char D_SEEN[] PROGMEM = "Seen:";

// ===== STATIONS ===== //
static const char ST_CLEARED_LIST[] PROGMEM = "Cleared station list";
static const char ST_REMOVED_STATION[] PROGMEM = "Removed station ";
static const char ST_LIST_EMPTY[] PROGMEM = "Station list is empty :(";
static const char ST_HEADER[] PROGMEM = "[===== Stations =====]";
static const char ST_NO_DEVICES_SELECTED[] PROGMEM = "No devices selected";
static const char ST_TABLE_HEADER[] PROGMEM =  "ID MAC               Ch Name             Vendor   Pkts     AP                               Last Seen Selected";
static const char ST_TABLE_DIVIDER[] PROGMEM = "==============================================================================================================";
static const char ST_SMALLER_ONESEC[] PROGMEM = "<1sec";
static const char ST_SMALLER_ONEMIN[] PROGMEM = "<1min";
static const char ST_BIGER_ONEHOUR[] PROGMEM = ">1h";
static const char ST_SELECTED_STATION[] PROGMEM = "Selected station ";
static const char ST_DESELECTED_STATION[] PROGMEM = "Deselected station ";
static const char ST_ERROR_ID[] PROGMEM = "ERROR: No station found with ID ";
static const char ST_SELECTED_ALL[] PROGMEM = "Selected all stations";
static const char ST_DESELECTED_ALL[] PROGMEM  = "Deselected all stations";

// ===== SETTINGS ===== //
static const char S_SETTINGS[] PROGMEM = "settings";
static const char S_BEACONCHANNEL[] PROGMEM = "beaconchannel";
static const char S_FORCEPACKETS[] PROGMEM = "forcepackets";
static const char S_AUTOSAVE[] PROGMEM = "autosave";
static const char S_LANG[] PROGMEM = "lang";
static const char S_SERIALINTERFACE[] PROGMEM = "serial";
static const char S_DISPLAYINTERFACE[] PROGMEM = "display";
static const char S_WEBINTERFACE[] PROGMEM = "web/interface";
static const char S_AUTOSAVETIME[] PROGMEM = "autosavetime";
//static const char S_DEAUTHSPERTARGET[] PROGMEM = "deauthspertarget";
static const char S_CHTIME[] PROGMEM = "chtime";
static const char S_MAXCH[] PROGMEM = "maxch";
static const char S_DEAUTHREASON[] PROGMEM = "deauthReason";
static const char S_MACST[] PROGMEM = "macSt";
static const char S_MACAP[] PROGMEM = "macAP";
static const char S_RANDOMTX[] PROGMEM = "randomTX";
static const char S_ATTACKTIMEOUT[] PROGMEM = "attacktimeout";
static const char S_LEDENABLED[] PROGMEM = "led/Enabled";
static const char S_PROBESPERSSID[] PROGMEM = "probesPerSSID";
static const char S_BEACONINTERVAL[] PROGMEM = "beaconInterval";
static const char S_VERSION[] PROGMEM = "version";
static const char S_CHANNEL[] PROGMEM = "channel";
static const char S_CAPTIVEPORTAL[] PROGMEM = "captivePortal";
static const char S_HIDDEN[] PROGMEM = "hidden";
static const char S_PASSWORD[] PROGMEM = "password";
static const char S_SSID[] PROGMEM = "ssid";
static const char S_MAC[] PROGMEM = "mac";
static const char S_MIN_DEAUTHS[] PROGMEM = "minDeauths";
static const char S_DISPLAY_TIMEOUT[] PROGMEM = "displayTimeout";

static const char S_SETTINGS_LOADED[] PROGMEM = "Settings loaded from ";
static const char S_SETTINGS_RESETED[] PROGMEM = "Settings reseted";
static const char S_SETTINGS_SAVED[] PROGMEM = "Settings saved in ";
static const char S_SETTINGS_HEADER[] PROGMEM = "[========== Settings ==========]";
static const char S_ERROR_VERSION[] PROGMEM = "Sorry, you can't change the version number";
static const char S_ERROR_NOT_FOUND[] PROGMEM = "ERROR: No setting found for ";
static const char S_CHANGED_SETTING[] PROGMEM = "Changed setting ";
static const char S_CHANNEL_CHANGE[] PROGMEM = "Switched to Channel ";
static const char S_CHANNEL_ERROR[] PROGMEM = "ERROR: Channel must be between 1 and ";
static const char S_ERROR_SSID_LEN[] PROGMEM = "ERROR: SSID must be between 1 and 32 characters";
static const char S_ERROR_PASSWORD_LEN[] PROGMEM = "ERROR: Password must be between 8 and 32 characters";
static const char S_RANDOM[] PROGMEM = "random";

// ===== ACCESS POINTS ===== //
static const char AP_HEADER[] PROGMEM = "[===== Access Points =====]";
static const char AP_LIST_EMPTY[] PROGMEM = "AP list is empty :(";
static const char AP_NO_AP_SELECTED[] PROGMEM = "No APs selected";
static const char AP_TABLE_HEADER[] PROGMEM =  "ID SSID                             Name             Ch RSSI Enc. Mac               Vendor   Selected";
static const char AP_TABLE_DIVIDER[] PROGMEM = "=====================================================================================================";
static const char AP_HIDDEN[] PROGMEM = "*HIDDEN*";
static const char AP_WEP[] PROGMEM = "WEP";
static const char AP_WPA[] PROGMEM = "WPA";
static const char AP_WPA2[] PROGMEM = "WPA2";
static const char AP_AUTO[] PROGMEM = "WPA*";
static const char AP_SELECTED[] PROGMEM = "Selected access point ";
static const char AP_DESELECTED[] PROGMEM = "Deselected access point ";
static const char AP_REMOVED[] PROGMEM = "Removed access point ";
static const char AP_SELECTED_ALL[] PROGMEM = "Selected all APs";
static const char AP_DESELECTED_ALL[] PROGMEM = "Deselected all APs";
static const char AP_REMOVED_ALL[] PROGMEM = "Removed all APs";
static const char AP_NO_AP_ERROR[] PROGMEM = "ERROR: No AP found with ID ";

// ===== ATTACKS ===== //
static const char A_START[] PROGMEM = "Start attacking";
static const char A_NO_MODE_ERROR[] PROGMEM = "WARNING: No valid attack mode set";
static const char A_STOP[] PROGMEM = "Stopped attacking";
static const char A_TIMEOUT[] PROGMEM = "Timeout - ";
static const char A_STATUS[] PROGMEM = "[Pkt/s] deauths: %+3u/%-3u beacons: %+3u/%-3u probes: %+3u/%-3u\r\n";
static const char A_ENABLED_OUTPUT[] PROGMEM = "Enabled attack output";
static const char A_DISABLED_OUTPUT[] PROGMEM = "Disabled attack output";

// ===== LED ===== //
static const char L_ENABLED[] PROGMEM = "Enabled LED updates";
static const char L_DISABLED[] PROGMEM = "Disabled LED updates";
static const char L_OUTPUT_A[] PROGMEM = "LED (%u,%u,%u) => ";
static const char L_OUTPUT_B[] PROGMEM = "(%u,%u,%u)\r\n";

// ===== NAMES ===== //
static const char N_SAVED[] PROGMEM = "Device names saved in ";
static const char N_LOADED[] PROGMEM = "Device names loaded from ";
static const char N_REMOVED_ALL[] PROGMEM = "Removed all saved device names";
static const char N_HEADER[] PROGMEM = "[===== Saved Devices =====]";
static const char N_TABLE_HEADER[] PROGMEM =  "ID MAC               Vendor   Name             AP-BSSID          Ch Selected";
static const char N_TABLE_DIVIDER[] PROGMEM = "============================================================================";
static const char N_ERROR_LIST_EMPTY[] PROGMEM = "Device name list is empty :(";
static const char N_ERROR_NO_SELECTED[] PROGMEM = "No devices selected";
static const char N_ERROR_LIST_FULL[] PROGMEM = "ERROR: Name list is full!";
static const char N_ADDED[] PROGMEM = "Added to device name list ";
static const char N_REPLACED[] PROGMEM = "Replaced device name list ";
static const char N_REMOVED[] PROGMEM = "Removed from the device name list ";
static const char N_CHANGED_NAME[] PROGMEM = "Changed device name";
static const char N_CHANGED_MAC[] PROGMEM = "Changed device mac";
static const char N_CHANGED_BSSID[] PROGMEM = "Changed device AP-BSSID";
static const char N_CHANGED_CH[] PROGMEM = "Changed device channel";
static const char N_SELECTED[] PROGMEM = "Selected device ";
static const char N_ERROR_NOT_FOUND[] PROGMEM = "No device found with name ";
static const char N_DESELECTED[] PROGMEM = "Deselected device ";
static const char N_SELECTED_ALL[] PROGMEM = "Selected all device names";
static const char N_DESELECTED_ALL[] PROGMEM = "Deselected all device names";

// ===== SSIDs ===== //
static const char SS_LOADED[] PROGMEM = "SSIDs loaded from ";
static const char SS_CLEARED[] PROGMEM = "Cleared SSID list";
static const char SS_SAVED[] PROGMEM = "SSIDs saved";
static const char SS_SAVED_IN[] PROGMEM = "SSIDs saved in ";
static const char SS_REMOVED[] PROGMEM = "Removed SSID ";
static const char SS_ERROR_FULL[] PROGMEM = "ERROR: SSID list is full! Remove some SSIDs first or run command with -f (force) parameter.";
static const char SS_ADDED[] PROGMEM = "Added SSID ";
static const char SS_REPLACED[] PROGMEM = "Replaced SSID ";
static const char SS_TABLE_HEADER[] PROGMEM = "ID Enc. SSID";
static const char SS_TABLE_DIVIDER[] PROGMEM = "=========================================";
static const char SS_HEADER[] PROGMEM = "[===== SSIDs =====]";
static const char SS_ERROR_EMPTY[] PROGMEM = "SSID list is empty :(";
static const char SS_RANDOM_ENABLED[] PROGMEM = "SSID random mode enabled";
static const char SS_RANDOM_DISABLED[] PROGMEM = "SSID random mode deactivated";
static const char SS_JSON_SSIDS[] PROGMEM = "ssids";
static const char SS_JSON_RANDOM[] PROGMEM = "random";
static const char SS_JSON_DEFAULT[] PROGMEM = "{\"random\":false,\"ssids\":[]}";
static const char SS_RANDOM_INFO[] PROGMEM = "Generating new SSIDs... Type \"disable random\" to stop the random mode";

// ===== SCAN ==== //
static const char SC_START_CLIENT[] PROGMEM = "Starting Scan for stations (client devices) - ";
static const char SS_START_SNIFFER[] PROGMEM = "Starting packet sniffer - ";
static const char SC_ERROR_NO_AP[] PROGMEM = "ERROR: AP-list empty! Can't scan for clients, please Scan for Accesspoints first.";
static const char SC_INFINITELY[] PROGMEM = " infinitely";
static const char SC_ON_CHANNEL[] PROGMEM = " on channel ";
static const char SC_START_AP[] PROGMEM = "Starting scan for access points (Wi-Fi networks)...";
static const char SC_ONE_TO[] PROGMEM = "1 - ";
static const char SC_STOPPED[] PROGMEM = "Stopped scan";
static const char SC_RESTRAT[] PROGMEM = "Scan will restart in ";
static const char SC_CONTINUE[] PROGMEM = "s - type stop to disable the continuous mode";
static const char SC_RESTART[] PROGMEM = "restarting in ";
static const char SC_ERROR_MODE[] PROGMEM = "ERROR: Invalid scan mode ";
static const char SC_OUTPUT_A[] PROGMEM = "Scanning WiFi [%+2u%%]: %+3u packets/s | %+2u devices | %+2u deauths\r\n";
static const char SC_OUTPUT_B[] PROGMEM = "Scanning WiFi: %+3u packets/s | %+2u devices | %+2u deauths\r\n";
static const char SC_JSON_APS[] PROGMEM = "aps";
static const char SC_JSON_STATIONS[] PROGMEM = "stations";
static const char SC_JSON_NAMES[] PROGMEM = "names";
static const char SC_SAVED[] PROGMEM = "Saved scan results";
static const char SC_SAVED_IN[] PROGMEM = "Scan results saved in ";

// ===== FUNCTIONS ===== //
static const char F_ERROR_MAC[] PROGMEM = "ERROR: MAC address invalid";
static const char F_COLOR_INVALID[] PROGMEM = "ERROR: Color code invalid";
static const char F_ERROR_READING_FILE[] PROGMEM = "ERROR: reading file ";
static const char F_LINE[] PROGMEM = "[%d] ";
static const char F_ERROR_FILE[] PROGMEM = "ERROR: File doesn't exist ";
static const char F_ERROR_OPEN[] PROGMEM = "ERROR couldn't open ";
static const char F_ERROR_PARSING_JSON[] PROGMEM = "ERROR parsing JSON ";
static const char F_ERROR_TO_BIG[] PROGMEM = "ERROR file too big ";
static const char F_TMP[] PROGMEM = "/tmp";
static const char F_COPY[] PROGMEM = "_copy";
static const char F_ERROR_SAVING[] PROGMEM = "ERROR: saving file. Try 'format' and restart - ";

// ===== WIFI ===== //
static const char W_STOPPED_AP[] PROGMEM = "Stopped Access Point";
static const char W_AP_REQUEST[] PROGMEM = "[AP] request: ";
static const char W_INDEX_HTML[] PROGMEM = "index.html";
static const char W_PWNED[] PROGMEM = "pwned";
static const char W_DEAUTHER[] PROGMEM = "deauth.me";
static const char W_WEBINTERFACE[] PROGMEM = "/web";
static const char W_DOWNLOAD[] PROGMEM = "download";
static const char W_STREAM[] PROGMEM = "application/octet-stream";
static const char W_HTML[] PROGMEM = "text/html";
static const char W_CSS[] PROGMEM = "text/css";
static const char W_JS[] PROGMEM = "application/javascript";
static const char W_PNG[] PROGMEM = "image/png";
static const char W_GIF[] PROGMEM = "image/gif";
static const char W_JPG[] PROGMEM = "image/jpeg";
static const char W_ICON[] PROGMEM = "image/x-icon";
static const char W_XML[] PROGMEM = "text/xml";
static const char W_XPDF[] PROGMEM = "application/x-pdf";
static const char W_XZIP[] PROGMEM = "application/x-zip";
static const char W_GZIP[] PROGMEM = "application/x-gzip";
static const char W_JSON[] PROGMEM = "application/json";
static const char W_TXT[] PROGMEM = "text/plain";
static const char W_DOT_HTM[] PROGMEM = ".htm";
static const char W_DOT_HTML[] PROGMEM = ".html";
static const char W_DOT_CSS[] PROGMEM = ".css";
static const char W_DOT_JS[] PROGMEM = ".js";
static const char W_DOT_PNG[] PROGMEM = ".png";
static const char W_DOT_GIF[] PROGMEM = ".gif";
static const char W_DOT_JPG[] PROGMEM = ".jpg";
static const char W_DOT_ICON[] PROGMEM = ".ico";
static const char W_DOT_XML[] PROGMEM = ".xml";
static const char W_DOT_PDF[] PROGMEM = ".pdf";
static const char W_DOT_ZIP[] PROGMEM = ".zip";
static const char W_DOT_GZIP[] PROGMEM = ".gz";
static const char W_DOT_JSON[] PROGMEM = ".json";
static const char W_NOT_FOUND[] PROGMEM = " NOT FOUND";
static const char W_OK[] PROGMEM = "OK";
static const char W_EDIT[] PROGMEM = "/edit";
static const char W_BAD_ARGS[] PROGMEM = "BAD ARGS";
static const char W_BAD_PATH[] PROGMEM = "BAD PATH";
static const char W_FILE_NOT_FOUND[] PROGMEM = "FileNotFound";
static const char W_OFF[] PROGMEM = "off";
static const char W_STARTED_AP[] PROGMEM = "Started AP";
static const char W_LIST[] PROGMEM = "/list";
static const char W_RUN[] PROGMEM = "/run";
static const char W_DEFAULT_LANG[] PROGMEM = "/lang/default.lang";
static const char W_CMD[] PROGMEM = "cmd";
static const char W_DIR[] PROGMEM = "dir";
static const char W_ERROR_PASSWORD[] PROGMEM = "ERROR: Password must have at least 8 characters!";
static const char W_STATUS_OUTPUT[] PROGMEM = "[WiFi] Path: '%s', Mode: '%s', SSID: '%s', password: '%s', channel: '%u', hidden: %s, captive-portal: %s\r\n";
static const char W_AP[] PROGMEM = "AP";
static const char W_STATION[] PROGMEM = "Station";
static const char W_MODE_OFF[] PROGMEM = "OFF";
static const char W_MODE_AP[] PROGMEM = "AP";
static const char W_MODE_ST[] PROGMEM = "STATION";
#endif

