#ifndef Settings_h
#define Settings_h

#include <EEPROM.h>
#include "Mac.h"
#include "MacList.h"
#include "NameList.h"

extern const bool debug;
extern String data_getVendor(uint8_t first,uint8_t second,uint8_t third);
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
#define attackEncryptedAdr 1095
#define ssidHiddenAdr 1096
#define apScanHiddenAdr 1097
#define apChannelAdr 1098
#define useLedAdr 1099
#define channelHopAdr 1100

class Settings
{
  public:
    Settings();
    void load();
    void reset();
    void save();
    String get();
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
    bool attackEncrypted;
    bool useLed;
    bool channelHop;
  private:
};

#endif
