#ifndef ClientScan_h
#define ClientScan_h

#define maxClientScanResults 40

#include <ESP8266WiFi.h>
#include "Mac.h"
#include "MacList.h"
#include "APScan.h"
#include "NameList.h"

extern "C" {
  #include "user_interface.h"
}

extern APScan apScan;
extern NameList nameList;

extern String data_getVendor(uint8_t first,uint8_t second,uint8_t third);
extern bool intInArray(int num, int _array[]);
extern void PrintHex8(uint8_t *data, uint8_t length);
extern const bool debug;

class ClientScan{
  public:
    ClientScan();

    void start(int _time);
    bool stop();
    void packetSniffer(uint8_t *buf, uint16_t len);
    
    String getResults();
    void select(int num);
    
    String getClientName(int num);
    int getClientPackets(int num);
    String getClientVendor(int num);
    Mac getClientMac(int num);
    bool getClientSelected(int num);
    int getClientConnectedAp(int num);
    int getFirstClient();

    int results = 0;
    int timeout = 0;
    
    bool sniffing = false;
  private:
    long startTime = 0;
    
    Mac from;
    Mac to;

    Mac broadcast;
    Mac zero;
    
    MacList clients;
    char vendors[maxClientScanResults][9];
    int packets[maxClientScanResults];
    bool selected[maxClientScanResults];
    int connectedToAp[maxClientScanResults];

    int channels[13];
    int channelsNum = 0;
    int curChannel = 0;
};

#endif
