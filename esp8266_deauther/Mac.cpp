#include "Mac.h"

Mac::Mac() {
  for (int i = 0; i < 6; i++) {
    adress[i] = 0x00;
  }
}

void Mac::set(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth, uint8_t fifth, uint8_t sixth) {
  adress[0] = first;
  adress[1] = second;
  adress[2] = third;
  adress[3] = fourth;
  adress[4] = fifth;
  adress[5] = sixth;
}

void Mac::set(uint8_t* mac) {
  for(int i=0; i<6 || i<sizeof(mac); i++){
    adress[i] = mac[i];
  }
}

void Mac::set(Mac adr) {
  for (int i = 0; i < 6; i++) {
    adress[i] = adr._get(i);
  }
}

void Mac::setAt(uint8_t first, int num) {
  if (num > -1 && num < 6) adress[num] = first;
}

bool Mac::valid() {
  for (int i = 0; i < 6; i++) {
    if (adress[i] != 0xFF && adress[i] != 0x00) return true;
  }
  return false;
}

String Mac::toString() {
  String value = "";
  for (int i = 0; i < 6; i++) {
    if (adress[i] < 0x10) {
      value += "0";
    }
    value += String(adress[i], HEX);
    if (i < 5) value += ":";
  }
  return value;
}

void Mac::_print() {
  Serial.print(Mac::toString());
}

void Mac::_println() {
  Serial.println(Mac::toString());
}

uint8_t* Mac::_get() {
  return adress;
}

uint8_t Mac::_get(int num) {
  return adress[num];
}

bool Mac::compare(Mac target) {
  for (int i = 0; i < 6; i++) {
    if (adress[i] != target._get(i)) return false;
  }
  return true;
}

void Mac::randomize() {
  uint8_t randomMac[6];
  getRandomVendorMac(randomMac);
  this->set(randomMac);
}


