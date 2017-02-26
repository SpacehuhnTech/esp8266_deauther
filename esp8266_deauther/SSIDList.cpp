#include "SSIDList.h"

SSIDList::SSIDList(){
  if(listAdr+SSIDListLength*SSIDLength > 4096) Serial.println("WARNING: EEPROM overflow!");
}

void SSIDList::load(){
  len = EEPROM.read(lenAdr);
  if( len < 0 ||len > SSIDListLength){
    clear();
    save();
  }
  for(int i=0;i<len;i++){
    for(int h=0;h<SSIDLength;h++){
      char _nextChar = EEPROM.read(listAdr+(i*SSIDLength)+h);
      names[i][h] = _nextChar;
    }
  }
}

void SSIDList::clear(){
  len = 0;
}

void SSIDList::add(String name){
  if(len < SSIDListLength){
    for(int i=0;i<SSIDLength;i++){
      if(i<name.length()) names[len][i] = name[i];
      else names[len][i] = 0x00;
    }
    len++;
  }
}

void SSIDList::edit(int num, String name){
  for(int i=0;i<SSIDLength;i++){
    if(i<name.length()) names[num][i] = name[i];
    else names[num][i] = 0x00;
  }
}

String SSIDList::get(int num){
  String _name = "";
  for(int i=0;i<SSIDLength;i++){
    if(names[num][i] != 0x00) _name += names[num][i];
  }
  return _name;
}

void SSIDList::remove(int num){
  if(num >= 0 && num < len){
    for(int i=num;i<len-1;i++){
      for(int h=0;h<SSIDLength;h++){
        names[i][h] = names[i+1][h];
      }
    }
    len--;
  }
}

void SSIDList::save(){
  if(debug) Serial.print("saving SSIDList...");
  EEPROM.write(lenAdr, len);
  for(int i=0;i<len;i++){
    for(int h=0;h<SSIDLength;h++){
      EEPROM.write(listAdr+(i*SSIDLength)+h, names[i][h]);
    }
  }
  EEPROM.commit();
  if(debug) Serial.println("done");
}

void SSIDList::_random(){
  String _rName;
  for(int i=len;i<SSIDListLength;i++){
    _rName = "";
    //_rName = "a"+(String)(int)random(0,99);
    for(int h=0; h < SSIDLength; h++) _rName += letters[random(0,sizeof(letters))];
    add(_rName);
  }
}



