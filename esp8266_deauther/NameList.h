#ifndef NameList_h
#define NameList_h

#include <EEPROM.h>
#include "Mac.h"
#include "MacList.h"

#define romAdr 0
#define maxSize 1024
#define listLength 50
#define nameLength 18

extern const bool debug;

class NameList
{
  public:
    NameList();
    void load();
    void clear();
    void add(Mac client, String name);
    void edit(int num, String name);
    String get(Mac client);
    String getName(int num);
    Mac getMac(int num);
    void remove(int num);
    int len = 0;
  private:
    void save();
    MacList clients;
    uint8_t names[listLength][nameLength];
};

#endif