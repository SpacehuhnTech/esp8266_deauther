#ifndef Mac_h
#define Mac_h

#include "Arduino.h"

class Mac
{
  public:
    Mac();
    void set(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth, uint8_t fifth, uint8_t sixth);
    void setAt(uint8_t first, int num);
    void setMac(Mac adr);
    String toString();
    void _print();
    void _println();
    uint8_t _get(int num);
    bool compare(Mac target);
    bool valid();
  private:
    uint8_t adress[6];
};

#endif
