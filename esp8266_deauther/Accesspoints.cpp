#include "Accesspoints.h"

Accesspoints::Accesspoints() {

}

void Accesspoints::sort() {
  // bubble sort because I was lazy
  
  AP* aAP; // prev
  AP* bAP; // to compare with c
  AP* cAP; // to compare with b
  AP* dAP; // next
  int c = listSize;
  while(c--){
    for(int i = 1; i <= c; i++){
      aAP = getAP(i-2); // prev
      bAP = aAP ? aAP->next : getAP(i-1); // to be compared
      cAP = bAP ? bAP->next : getAP(i);  // to be compared
      dAP = cAP ? cAP->next : getAP(i+1);  // next

      // a -> b -> c -> d
      
      if(WiFi.RSSI(bAP->id) < WiFi.RSSI(cAP->id)) {
        // a -> b <-> c
        cAP->next = bAP;

        // a -> b -> d
        if(dAP){
          bAP->next = dAP;
        } else {
          bAP->next = NULL;
          listEnd = bAP;
        }

        // a -> c -> b -> d
        if(aAP)
          aAP->next = cAP;
        else
          listBegin = cAP;
      }
    }
  }
}

void Accesspoints::sortAfterChannel() {
  AP* aAP;
  AP* bAP;
  AP* cAP;
  AP* dAP;
  int c = listSize;
  while(c--){
    for(int i = 1; i <= c; i++){
      aAP = getAP(i-2);
      bAP = aAP ? aAP->next : getAP(i-1);
      cAP = bAP ? bAP->next : getAP(i);
      dAP = cAP ? cAP->next : getAP(i+1);
      if(WiFi.channel(bAP->id) > WiFi.channel(cAP->id)) {
        cAP->next = bAP;
        
        if(dAP){
          bAP->next = dAP;
        } else {
          bAP->next = NULL;
          listEnd = bAP;
        }

        if(aAP) aAP->next = cAP;
        else listBegin = cAP;
      }
    }
  }
  changed = true;
}

void Accesspoints::add(uint8_t id, bool selected) {
  if(!listEnd){
    listEnd = new AP{id, selected};
  } else {
    listEnd->next = new AP{id, selected};
    listEnd = listEnd->next;
  }
    
  if(!listBegin)
    listBegin = listEnd;

  listSize++;
  changed = true;
}

void Accesspoints::printAll() {
  prntln(AP_HEADER);
  int c = count();
  if (c == 0)
    prntln(AP_LIST_EMPTY);
  else
    for (int i = 0; i < c; i++)
      print(i, i == 0, i == c - 1);
}

void Accesspoints::printSelected() {
  prntln(AP_HEADER);
  int max = selected();
  if (selected() == 0) {
    prntln(AP_NO_AP_SELECTED);
    return;
  }
  int c = count();
  for (int i = 0, j = 0; i < c, j < max; i++) {
    if (getSelected(i)) {
      print(i, j == 0, j == max - 1);
      j++;
    }
  }
}

void Accesspoints::print(int num) {
  print(num, true, true);
}

void Accesspoints::print(int num, bool header, bool footer) {
  if (!check(num)) return;
  if (header) {
    prntln(AP_TABLE_HEADER);
    prntln(AP_TABLE_DIVIDER);
  }
  prnt(buildString(String(),(String)num, 2));
  prnt(buildString(String(SPACE) + getSSID(num), String(), 33));
  prnt(buildString(String(SPACE) + getNameStr(num), String(), 17));
  prnt(buildString(String(SPACE), (String)getCh(num), 3));
  prnt(buildString(String(SPACE), (String)getRSSI(num), 5));
  prnt(buildString(String(SPACE), getEncStr(num), 5));
  prnt(buildString(String(SPACE) + getMacStr(num), String(), 18));
  prnt(buildString(String(SPACE) + getVendorStr(num), String(), 9));
  prntln(buildString(String(SPACE) + getSelectedStr(num), String(), 9));
  
  if (footer) {
    prntln(AP_TABLE_DIVIDER);
  }
}

String Accesspoints::getSSID(int num) {
  if (!check(num)) return String();
  if (getHidden(num)){
    return str(AP_HIDDEN);
  } else {
    String ssid = WiFi.SSID(getID(num));
    ssid = ssid.substring(0,32);
    ssid = fixUtf8(ssid);
    return ssid;
  }
}

String Accesspoints::getNameStr(int num) {
  if (!check(num)) return String();
  return names.find(getMac(num));
}

uint8_t Accesspoints::getCh(int num) {
  if (!check(num)) return 0;
  return WiFi.channel(getID(num));
}

int Accesspoints::getRSSI(int num) {
  if (!check(num)) return 0;
  return WiFi.RSSI(getID(num));
}

uint8_t Accesspoints::getEnc(int num) {
  if (!check(num)) return 0;
  return WiFi.encryptionType(getID(num));
}

String Accesspoints::getEncStr(int num) {
  if (!check(num)) return String();
  switch (getEnc(num)) {
    case ENC_TYPE_NONE:
      return String(DASH);
      break;
    case ENC_TYPE_WEP:
      return str(AP_WEP);
      break;
    case ENC_TYPE_TKIP:
      return str(AP_WPA);
      break;
    case ENC_TYPE_CCMP:
      return str(AP_WPA2);
      break;
    case ENC_TYPE_AUTO:
      return str(AP_AUTO);
      break;
  }
  return String(QUESTIONMARK);
}

String Accesspoints::getSelectedStr(int num) {
  return b2a(getSelected(num));
}

uint8_t* Accesspoints::getMac(int num) {
  if (!check(num)) return 0;
  return WiFi.BSSID(getID(num));
}

String Accesspoints::getMacStr(int num) {
  if (!check(num)) return String();
  uint8_t* mac = getMac(num);
  String value;
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) value += ZERO;
    value += String(mac[i], HEX);
    if (i < 5) value += DOUBLEPOINT;
  }
  return value;
}

String Accesspoints::getVendorStr(int num) {
  if (!check(num)) return String();
  return searchVendor(getMac(num));
}

bool Accesspoints::getHidden(int num) {
  if (!check(num)) return false;
  return WiFi.isHidden(getID(num));
}

bool Accesspoints::getSelected(int num) {
  if (!check(num)) return false;
  return getAP(num)->selected;
}

int Accesspoints::getID(int num){
  if (!check(num)) return -1;
  return getAP(num)->id;
}

void Accesspoints::select(int num) {
  if (!check(num)) return;

  internal_select(num);
  
  prnt(AP_SELECTED);
  prntln(getSSID(num));
  
  changed = true;
}

void Accesspoints::deselect(int num) {
  if (!check(num)) return;

  internal_deselect(num);
  
  prnt(AP_DESELECTED);
  prntln(getSSID(num));

  changed = true;
}

void Accesspoints::remove(int num) {
  if (!check(num)) return;
  
  prnt(AP_REMOVED);
  prntln(getSSID(num));

  internal_remove(num);
  
  changed = true;
}

void Accesspoints::selectAll() {
  int i = 0;
  AP* hAP = listBegin;
  while(i < listSize){
    hAP->selected = true;
    hAP = hAP->next;
    i++;
  }
  prntln(AP_SELECTED_ALL);
  changed = true;
}

void Accesspoints::deselectAll() {
  int i = 0;
  AP* hAP = listBegin;
  while(i < listSize){
    hAP->selected = false;
    hAP = hAP->next;
    i++;
  }
  prntln(AP_DESELECTED_ALL);
  changed = true;
}

void Accesspoints::removeAll() {
  while(listSize > 0){
    internal_remove(0);
  }
  prntln(AP_REMOVED_ALL);
  changed = true;
}

int Accesspoints::count() {
  return listSize;
}

int Accesspoints::selected() {
  int num = 0;
  int i = 0;
  AP* hAP = listBegin;
  while(i < listSize){
    num += hAP->selected;
    hAP = hAP->next;
    i++;
  }
  return num;
}

bool Accesspoints::check(int num) {
  if (internal_check(num)) return true;
  prnt(AP_NO_AP_ERROR);
  prntln((String)num);
  return false;
}

AP* Accesspoints::getAP(int num){
  if(num < 0 || num >= listSize) return NULL;
  
  AP* hAP = listBegin;
  int i = 0;
  while(hAP->next && i < num){
    hAP = hAP->next;
    i++;
  }

  return hAP;
}

bool Accesspoints::internal_check(int num) {
  return num >= 0 && num < count();
}

void Accesspoints::internal_select(int num) {
  getAP(num)->selected = true;
}

void Accesspoints::internal_deselect(int num) {
  getAP(num)->selected = false;
}

void Accesspoints::internal_remove(int num) {
  AP* aAP = getAP(num-1); // prev
  AP* bAP = aAP ? aAP->next : getAP(num); // to-delete
  AP* cAP = bAP ? bAP->next : getAP(num+1); // next
  
  if(aAP && cAP) { // a -> b -> c = a -> c
    aAP->next = cAP; // 
  } else if(aAP) { // a -> b = a
    aAP->next = NULL;
    listEnd = aAP;
  } else if(cAP) { // b -> c = c
    listBegin = cAP;
  } else { // b = EMPTY
    listBegin = NULL;
    listEnd = NULL;
  }
  
  free(bAP);

  listSize--;
}

