#ifndef Accesspoints_h
#define Accesspoints_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "Names.h"
#include "language.h"

extern Names names;

extern String searchVendor(uint8_t* mac);
extern String buildString(String left, String right, int maxLen);
String fixUtf8(String str);

struct AP{
  uint8_t id;
  bool selected;
  AP* next;
};

class Accesspoints {
  public:
    Accesspoints();
    
    void sort();
    void sortAfterChannel();
    
    void add(uint8_t id, bool selected);
    void print(int num);
    void print(int num, bool header, bool footer);
    void select(int num);
    void deselect(int num);
    void remove(int num);

    void printAll();
    void printSelected();
    void selectAll();
    void deselectAll();
    void removeAll();
    
    String getSSID(int num);
    String getNameStr(int num);
    String getEncStr(int num);
    String getMacStr(int num);
    String getVendorStr(int num);
    String getSelectedStr(int num);
    uint8_t getCh(int num);
    uint8_t getEnc(int num);
    int getID(int num);
    int getRSSI(int num);
    uint8_t* getMac(int num);    
    bool getHidden(int num);
    bool getSelected(int num);

    int count();
    int selected();

    bool check(int num);
    bool changed = false;
  private:
    AP* listBegin = NULL;
    AP* listEnd = NULL;
    int listSize = 0;
    //LinkedList<AP>* listA;

    AP* getAP(int num);
    
    bool internal_check(int num);
    void internal_select(int num);
    void internal_deselect(int num);
    void internal_remove(int num);
};

#endif



