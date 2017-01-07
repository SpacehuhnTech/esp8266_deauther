#include "Mac.h"

Mac::Mac(){
  adress[0] = 0x00;
  adress[1] = 0x00;
  adress[2] = 0x00;
  adress[3] = 0x00;
  adress[4] = 0x00;
  adress[5] = 0x00;
}

void Mac::set(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth, uint8_t fifth, uint8_t sixth){
  adress[0] = first;
  adress[1] = second;
  adress[2] = third;
  adress[3] = fourth;
  adress[4] = fifth;
  adress[5] = sixth;
}

void Mac::setAt(uint8_t first, int num){
  if(num > -1 && num < 6) adress[num] = first;
}

void Mac::setMac(Mac adr){
  adress[0] = adr._get(0);
  adress[1] = adr._get(1);
  adress[2] = adr._get(2);
  adress[3] = adr._get(3);
  adress[4] = adr._get(4);
  adress[5] = adr._get(5);
}

bool Mac::valid(){
  for(int i=0;i<6;i++){
    if(adress[i] != 0xFF && adress[i] != 0x00) return true;
  }
  return false;
}

String Mac::toString(){
  String value = "";
  for(int i=0; i<6; i++) {
    if(adress[i]<0x10) {
      value += "0";
    }
    value += String(adress[i],HEX);
    if(i<5) value += ":";
  }
  return value;
}

void Mac::_print(){
  Serial.print(Mac::toString());
}

void Mac::_println(){
  Serial.println(Mac::toString());
}

uint8_t Mac::_get(int num){
  return adress[num];
}

bool Mac::compare(Mac target){
  for(int i=0;i<6;i++){
    if(adress[i] != target._get(i)) return false;
  }
  return true;
}


