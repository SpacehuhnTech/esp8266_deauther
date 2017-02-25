#ifndef NameList_h
#define NameList_h

#include <EEPROM.h>
#include "Mac.h"
#include "MacList.h"

#define romAdr 0
#define maxSize 1024
#define listLength 30
#define nameLength 18

/*
The NameList holds and saves all your custom device names in the EEPROM.
You can modify the length above, but be careful the EEPROM size is limited.
You may have to call nameList.clear() when uploading for the first time.
*/
extern const bool debug;

class NameList
{
  public:
    NameList();
    void load();
    void clear();
    void add(Mac client, String name);
    String get(Mac client);
  private:
    void save();

    int len = 0;
    MacList clients;
    uint8_t names[listLength][nameLength];
};

#endif
