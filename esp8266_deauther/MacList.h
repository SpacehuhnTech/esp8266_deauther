#ifndef MacList_h
#define MacList_h

#define listLen 80

#include "Mac.h"

class MacList
{
  public:
    MacList();
    int add(Mac adr);
    void remove(Mac adr);
    void info();
    bool contains(Mac adr);
    int getNum(Mac adr);
    Mac _get(int i);
    void _clear();
    
    int num = 0;
    Mac macAdrs[listLen];
  private:
    int len = listLen;
    
    void addPacket(Mac adr);
};

#endif
