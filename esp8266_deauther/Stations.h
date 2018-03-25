#ifndef Stations_h
#define Stations_h

#include "Arduino.h"
#include "LinkedList.h"
#include "Accesspoints.h"
#include "Names.h"
#include "language.h"

extern "C" {
  #include "user_interface.h"
}

#define STATION_LIST_SIZE 60
extern Accesspoints accesspoints;
extern Names names;
extern uint8_t wifi_channel;
extern uint32_t currentTime;

extern String searchVendor(uint8_t* mac);
extern bool macMulticast(uint8_t* mac);
extern bool macValid(uint8_t* mac);
extern bool macBroadcast(uint8_t* mac);

class Stations {
  public:
    Stations();
    
    void sort();
    void sortAfterChannel();
    
    void select(int num);
    void deselect(int num);
    void add(uint8_t* mac,int accesspointNum);
    
    void selectAll();
    void deselectAll();
    void removeAll();
    void remove(int num);
    void removeOldest();
    
    String getNameStr(int num);
    String getAPStr(int num);
    String getMacStr(int num);
    String getMacVendorStr(int num);
    String getVendorStr(int num);
    String getTimeStr(int num);
    String getSelectedStr(int num);
    uint8_t* getMac(int num);
    uint32_t* getPkts(int num);
    uint32_t* getTime(int num);
    uint8_t getCh(int num);
    uint8_t getAP(int num);
    bool getSelected(int num);
    bool hasName(int num);

    void print(int num);
    void print(int num, bool header, bool footer);
    void printAll();
    void printSelected();

    int count();
    int selected();

    bool check(int num);
    bool changed = false;
  private:
    struct Station{
      uint8_t ap;
      uint8_t ch;
      uint8_t* mac;
      uint32_t* pkts;
      uint32_t* time;
      bool selected;
    };

    LinkedList<Station>* list;

    int findStation(uint8_t* mac);
    int findAccesspoint(uint8_t* mac);

    bool internal_check(int num);
    void internal_select(int num);
    void internal_deselect(int num);
    void internal_add(uint8_t* mac, int accesspointNum);
    void internal_remove(int num);
    void internal_removeAll();
};

#endif



