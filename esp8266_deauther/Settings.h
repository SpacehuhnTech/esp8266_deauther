#ifndef Settings_h
#define Settings_h

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Mac.h"
#include "MacList.h"
#include "NameList.h"

extern "C" {
  #include "user_interface.h"
}

extern void sendBuffer();
extern void sendToBuffer(String str);
extern void sendHeader(int code, String type, size_t _size);

extern const bool debug;
extern String data_getVendor(uint8_t first, uint8_t second, uint8_t third);
extern void eepromWriteInt(int adr, int val);
extern int eepromReadInt(int adr);
extern NameList nameList;

#define ssidLenAdr 1024
#define ssidAdr 1025
#define passwordLenAdr 1057
#define passwordAdr 1058
#define deauthReasonAdr 1090
#define attackTimeoutAdr 1091
#define attackPacketRateAdr 1093
#define clientScanTimeAdr 1094
//#define attackEncryptedAdr 1095 <-- address is now free for another setting
#define ssidHiddenAdr 1096
#define apScanHiddenAdr 1097
#define apChannelAdr 1098
#define useLedAdr 1099
#define channelHopAdr 1100
#define multiAPsAdr 1101
#define multiAttacksAdr 1102
#define macIntervalAdr 1103
#define beaconIntervalAdr 1105
#define ledPinAdr 1106
#define macAPAdr 1107
#define isMacAPRandAdr 1113

#define checkNumAdr 2001
#define checkNum 16

class Settings
{
  public:
    Settings();
    void load();
    void reset();
    void save();
    void send();
    void info();

    int ssidLen;
    String ssid = "";
    bool ssidHidden;
    int passwordLen;
    String password = "";
    int apChannel;
    bool apScanHidden;
    uint8_t deauthReason;
    unsigned int attackTimeout;
    int attackPacketRate;
    int clientScanTime;
    bool useLed;
    bool channelHop;
    bool multiAPs;
    bool multiAttacks;
    int macInterval;
    bool beaconInterval;
    int ledPin = 0;
    int prevLedPin = 0;
    Mac defaultMacAP;
    Mac macAP;
    bool isMacAPRand;
    bool isSettingsLoaded = 0;
    void syncMacInterface();
    void setLedPin(int newLedPin);
    bool pinStateOff = true;  // When attack is off, pin state is HIGH
    
  private:
    size_t getSize();
};

#endif
