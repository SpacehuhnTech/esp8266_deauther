/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#pragma once

#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}
#include "src/ArduinoJson-v5.13.5/ArduinoJson.h"
#include "language.h"
#include "SimpleList.h"

#define NAME_LIST_SIZE 25
#define NAME_MAX_LENGTH 17

extern void checkFile(String path, String data);
extern JsonVariant parseJSONFile(String path, DynamicJsonBuffer& jsonBuffer);
extern bool writeFile(String path, String& buf);
extern bool appendFile(String path, String& buf);
extern bool strToMac(String macStr, uint8_t* mac);
extern String searchVendor(uint8_t* mac);
extern String fixUtf8(String str);
extern String leftRight(String a, String b, int len);
extern String escape(String str);
extern String bytesToStr(const uint8_t* b, uint32_t size);

class Names {
    public:
        Names();

        void load();
        void load(String filepath);
        void save(bool force);
        void save(bool force, String filepath);
        void sort();

        String find(uint8_t* mac);
        int findID(uint8_t* mac);

        void print(int num);
        void print(int num, bool header, bool footer);
        void select(int num);
        void select(String name);
        void deselect(int num);
        void deselect(String name);
        void add(uint8_t* mac, String name, uint8_t* bssid, uint8_t ch, bool selected, bool force);
        void add(String macStr, String name, String bssidStr, uint8_t ch, bool selected, bool force);
        void replace(int num, String macStr, String name, String bssidStr, uint8_t ch, bool selected);
        void remove(int num);

        void printAll();
        void printSelected();
        void selectAll();
        void deselectAll();
        void removeAll();

        uint8_t* getMac(int num);
        uint8_t* getBssid(int num);
        String getMacStr(int num);
        String getBssidStr(int num);
        String getName(int num);
        String getVendorStr(int num);
        String getSelectedStr(int num);
        uint8_t getCh(int num);
        bool getSelected(int num);
        bool isStation(int num);

        void setName(int num, String name);
        void setMac(int num, String macStr);
        void setCh(int num, uint8_t ch);
        void setBSSID(int num, String bssidStr);

        int count();
        int selected();
        int stations();

        bool check(int num);

    private:
        String FILE_PATH = "/names.json";
        bool changed     = false;

        struct Device {
            uint8_t* mac;      // mac address
            char   * name;     // name of saved device
            uint8_t* apBssid;  // mac address of AP (if saved device is a station)
            uint8_t  ch;       // Wi-Fi channel of Device
            bool     selected; // select for attacking
        };

        SimpleList<Device>* list;

        int binSearch(uint8_t* searchBytes, int lowerEnd, int upperEnd);
        bool internal_check(int num);
        void internal_select(int num);
        void internal_deselect(int num);
        void internal_add(uint8_t* mac, String name, uint8_t* bssid, uint8_t ch, bool selected);
        void internal_add(String macStr, String name, String bssidStr, uint8_t ch, bool selected);
        void internal_remove(int num);
        void internal_removeAll();
};