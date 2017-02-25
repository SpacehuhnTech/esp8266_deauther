#ifndef Settings_h
#define Settings_h

#include <EEPROM.h>
#include "Mac.h"
#include "MacList.h"

extern const bool debug;
extern void eepromWriteInt(int adr, int val);
extern int eepromReadInt(int adr);

#define ssidLenAdr 1024
#define ssidAdr 1025
#define passwordLenAdr 1057
#define passwordAdr 1058
#define deauthReasonAdr 1090
#define attackTimeoutAdr 1091
#define attackPacketRateAdr 1093
#define attackMacIntervalAdr 1094
#define clientScanTimeAdr 1095

class Settings
{
  public:
    Settings();
    void load();
    void reset();
    void save();
    void info();

    uint8_t ssidLen;
    String ssid;
    uint8_t passwordLen;
    String password;
    
    uint8_t deauthReason;
    int attackTimeout;
    int attackPacketRate;
    int attackMacInterval;
    
    int clientScanTime;
  private:
};

#endif
