#ifndef SerialInterface_h
#define SerialInterface_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <FS.h>
extern "C" {
  #include "user_interface.h"
}
#include "language.h"
#include "A_config.h"
#include "SimpleList.h"
#include "Settings.h"
#include "Names.h"
#include "SSIDs.h"
#include "Scan.h"
#include "Attack.h"
#include "DisplayUI.h"
#include "LEDController.h"

extern LEDController* led;
extern Settings settings;
extern Names    names;
extern SSIDs    ssids;
extern Accesspoints accesspoints;
extern Stations     stations;
extern Scan   scan;
extern Attack attack;
extern DisplayUI displayUI;
extern uint32_t  currentTime;
extern uint32_t  autosaveTime;

extern String macToStr(uint8_t* mac);
extern void strToColor(String str, uint8_t* buf);
extern void readFileToSerial(String path, bool showLineNum);
extern bool removeFile(String path);
extern bool copyFile(String pathFrom, String pathTo);
extern bool renameFile(String pathFrom, String pathTo);
extern bool appendFile(String path, String& buf);
extern bool removeLines(String path, int lineFrom, int lineTo);
extern bool replaceLine(String path, int line, String& buf);
extern bool equalsKeyword(const char* str, const char* keyword);
extern void printWifiStatus();
extern void startAP(String path, String ssid, String password, uint8_t ch, bool hidden, bool captivePortal);
extern void wifiUpdate();

class SerialInterface {
    public:
        SerialInterface();
        void enable();
        void load();
        void load(String filepath);
        void disable();
        void update();
        void stopScript();

        void runCommands(String input);
        void runCommand(String input);

        void error(String message);
        void parameterError(String parameter);

    private:
        bool enabled;
        SimpleList<String>* list;
        bool executing        = false;
        bool continuously     = false;
        uint32_t continueTime = 0;
        uint32_t loopTime     = 0;
        String execPath       = "/autostart.txt";

        struct Keyword {
            const char* name;
            const char* shortName;
            const char* alt;
        };

        bool isInt(String str);
        int toInt(String str);
        uint32_t getTime(String time);
        bool eqlsCMD(int i, const char* keyword);
};

#endif // ifndef SerialInterface_h