#ifndef Mac_h
#define Mac_h

#include <Arduino.h>

extern void getRandomVendorMac(uint8_t *buf);

class Mac
{
  public:
    Mac();
    void set(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth, uint8_t fifth, uint8_t sixth);
    void set(uint8_t* mac);
    void set(Mac adr);
    void setAt(uint8_t first, int num);
    String toString();
    void _print();
    void _println();
    uint8_t* _get();
    uint8_t _get(int num);
    bool compare(Mac target);
    bool valid();
    void randomize();
  private:
    uint8_t adress[6];
};

#endif
