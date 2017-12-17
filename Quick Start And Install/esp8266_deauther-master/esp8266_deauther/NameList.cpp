#include "NameList.h"

NameList::NameList() {
  if ((listLength * nameLength + 6) + 1 > maxSize) Serial.println("ERROR: EEPROM OVERFLOW!");
}

void NameList::load() {
  len = (int)EEPROM.read(romAdr);
  if (len < 1 || len > listLength) NameList::clear();

  int num = 0;
  Mac _client;

  for (int i = 1; i < len * (nameLength + 6) + 1; i += nameLength + 6) {
    for (int h = 0; h < 6; h++) _client.setAt(EEPROM.read(i + h), h);
    for (int h = 0; h < nameLength; h++) names[num][h] = EEPROM.read(i + h + 6);
    clients.add(_client);
    num++;
  }
}

void NameList::clear() {
  len = 0;
  clients._clear();
  EEPROM.write(romAdr, len);
  EEPROM.commit();
  Serial.println("nameList cleared");
}

void NameList::save() {
  EEPROM.write(romAdr, len);
  int num = 0;
  for (int i = 1; i < len * (nameLength + 6) + 1; i += nameLength + 6) {
    for (int h = 0; h < 6; h++) EEPROM.write(i + h, clients._get(num)._get(h));
    for (int h = 0; h < nameLength; h++) EEPROM.write(i + h + 6, names[num][h]);
    num++;
  }
  EEPROM.commit();
}

void NameList::add(Mac client, String name) {
  if (clients.add(client) >= 0) len++;
  else if (clients.getNum(client) < 0) Serial.println("WARNING: name list is full!");
  uint8_t _buf[nameLength];
  name.getBytes(_buf, nameLength);
  for (int i = 0; i < nameLength; i++) {
    if (i < name.length()) names[clients.getNum(client)][i] = _buf[i];
    else names[clients.getNum(client)][i] = 0x00;
  }
  NameList::save();
}

String NameList::get(int num) {
  String returnStr;
  if (num >= 0) {
    for (int h = 0; h < nameLength; h++) {
      if (names[num][h] != 0x00) returnStr += (char)names[num][h];
    }
    returnStr.trim();
    return returnStr;
  }
}

Mac NameList::getMac(int num) {
  return clients._get(num);
}

String NameList::getByMac(Mac client) {
  String returnStr;
  int clientNum = clients.getNum(client);
  if (clientNum > -1) {
    for (int h = 0; h < nameLength; h++) {
      if (names[clientNum][h] != 0x00) returnStr += (char)names[clientNum][h];
    }
  }
return returnStr;
}

int NameList::getNumByMac(Mac client) {
  return clients.getNum(client);
}

void NameList::remove(int num) {
  for (int i = num; i < len - 1; i++) {
    clients.set(num, clients._get(num + 1));
    for (int h = 0; h < nameLength; h++) names[num][h] = names[num + 1][h];
  }
  clients.remove(clients._get(len));
  clients.num--;
  len--;
  save();
}

void NameList::edit(int num, String name) {
  for (int i = 0; i < nameLength; i++) {
    if (i < name.length()) names[num][i] = name[i];
    else names[num][i] = 0x00;
  }
}

