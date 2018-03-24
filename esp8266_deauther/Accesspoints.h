#ifndef Accesspoints_h
#define Accesspoints_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <LinkedList.h>
#include "Names.h"
#include "language.h"

extern Names names;

extern String searchVendor(uint8_t* mac);
extern String buildString(String left, String right, int maxLen);
String fixUtf8(String str);

class Accesspoints {
  public:
    Accesspoints();
    
    void sort();
    void sortAfterChannel();
    
    void add(uint8_t id, bool selected);
    void print(uint8_t num);
    void print(uint8_t num, bool header, bool footer);
    void select(uint8_t num);
    void deselect(uint8_t num);
    void remove(uint8_t num);

    void printAll();
    void printSelected();
    void selectAll();
    void deselectAll();
    void removeAll();
    
    String getSSID(uint8_t num);
    String getNameStr(uint8_t num);
    String getEncStr(uint8_t num);
    String getMacStr(uint8_t num);
    String getVendorStr(uint8_t num);
    String getSelectedStr(uint8_t num);
    uint8_t getCh(uint8_t num);
    uint8_t getEnc(uint8_t num);
    int8_t getRSSI(uint8_t num);
    uint8_t* getMac(uint8_t num);    
    bool getHidden(uint8_t num);
    bool getSelected(uint8_t num);

    uint8_t count();
    uint8_t selected();

    bool check(uint8_t num);
    bool changed = false;
  private:
    struct AP{
      uint8_t id;
      bool selected;
    };

    LinkedList<AP>* list;

    bool internal_check(uint8_t num);
    void internal_select(uint8_t num);
    void internal_deselect(uint8_t num);
    void internal_remove(uint8_t num);
};

#endif



