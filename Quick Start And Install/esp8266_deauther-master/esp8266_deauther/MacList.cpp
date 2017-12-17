#include "MacList.h"

MacList::MacList() {

}

bool MacList::contains(Mac adr) {
  for (int i = 0; i < num; i++) {
    if (macAdrs[i].compare(adr)) return true;
  }
  return false;
}

int MacList::add(Mac adr) {
  if(num < len && adr.valid()) {
    if (!contains(adr)) {
      macAdrs[num].set(adr);
      num++;
      return num - 1;
    }
  }
  return -1;
}

Mac MacList::_get(int i) {
  return macAdrs[i];
}

int MacList::getNum(Mac adr) {
  for (int i = 0; i < num; i++) {
    if (macAdrs[i].compare(adr)) {
      return i;
    }
  }
  return -1;
}

void MacList::info() {
  Serial.print("Items: " + (String)num + " / " + (String)len + "\n");
  for (int i = 0; i < num; i++) {
    macAdrs[i]._print();
  }
}

void MacList::_clear() {
  for (int i = 0; i < num; i++) {
    macAdrs[i].set(0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  }
  num = 0;
}

void MacList::remove(Mac adr) {
  for (int i = 0; i < num; i++) {
    if (macAdrs[i].compare(adr)) {
      macAdrs[i].set(0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    }
  }
}

void MacList::set(int num, Mac adr) {
  macAdrs[num].set(adr);
}

