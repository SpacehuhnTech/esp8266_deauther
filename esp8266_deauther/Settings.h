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
#define clientScanTimeAdr 1094

class Settings
{
  public:
    Settings();
    void load();
    void reset();
    void save();
    void info();

    int ssidLen;
    String ssid = "";
    int passwordLen;
    String password = "";
    
    uint8_t deauthReason;
    int attackTimeout;
    int attackPacketRate;
    
    int clientScanTime;
  private:
};

#endif
