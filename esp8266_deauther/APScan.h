#ifndef APScan_h
#define APScan_h

#define maxResults 80

#include <ESP8266WiFi.h>
#include "Mac.h"
#include "MacList.h"

extern String data_getVendor(uint8_t first,uint8_t second,uint8_t third);
extern const bool debug;

class APScan{
  public:
    APScan();
    
    bool start();
    String getResult(int i);
    String getResults();
    int getResultByAPName(String apName);
    int select(int num);
    bool setAsyncIndex();
    String getAPName(int num);
    String getAPEncryption(int num);
    String getAPVendor(int num);
    String getAPMac(int num);
    String getAPSelected(int num);
    int getAPRSSI(int num);
    int getAPChannel(int num);

    int getFirstTarget();
    bool isSelected(int num);
    
    int results = 0;
    int selectedSum;
    MacList aps;
    int asyncIndex = -1;
  private:
    int channels[maxResults];
    int rssi[maxResults];
    char names[maxResults][33];
    char encryption[maxResults][5];
    char vendors[maxResults][9];

    String getEncryption(int code);

    bool selected[maxResults];
};

#endif
