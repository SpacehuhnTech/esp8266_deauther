/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#pragma once

#include "Arduino.h"
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}
#include "src/ArduinoJson-v5.13.5/ArduinoJson.h"
#include "language.h"
#include "SimpleList.h"
#include "Accesspoints.h"

#define SSID_LIST_SIZE 60


extern uint32_t currentTime;
extern Accesspoints accesspoints;

extern void checkFile(String path, String data);
extern JsonVariant parseJSONFile(String path, DynamicJsonBuffer& jsonBuffer);
extern bool appendFile(String path, String& buf);
extern bool writeFile(String path, String& buf);
extern void readFileToSerial(String path);
extern String fixUtf8(String str);
extern String leftRight(String a, String b, int len);
extern String escape(String str);

class SSIDs {
    public:
        SSIDs();

        void load();
        void load(String filepath);
        void save(bool force);
        void save(bool force, String filepath);
        void update();

        void print(int num);
        void print(int num, bool header, bool footer);
        void add(String name, bool wpa2, int clones, bool force);
        void cloneSelected(bool force);
        void remove(int num);
        void enableRandom(uint32_t randomInterval);
        void disableRandom();
        bool getRandom();

        String getName(int num);
        bool getWPA2(int num);
        String getEncStr(int num);
        int getLen(int num);

        void setWPA2(int num, bool wpa2);
        void replace(int num, String name, bool wpa2);

        void printAll();
        void removeAll();

        int count();

    private:
        bool changed            = false;
        bool randomMode         = false;
        uint32_t randomInterval = 2000;
        uint32_t randomTime     = 0;

        struct SSID {
            String  name; // SSID
            bool    wpa2; // WPA2 encrypted or not
            uint8_t len;  // original length (before editing it to be 32 characters)
        };

        String FILE_PATH = "/ssids.json";

        SimpleList<SSID>* list;

        bool check(int num);
        String randomize(String name);

        void internal_add(String name, bool wpa2, int add);
        void internal_remove(int num);
        void internal_removeAll();
};