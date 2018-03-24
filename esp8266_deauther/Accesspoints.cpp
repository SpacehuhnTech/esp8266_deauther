#include "Accesspoints.h"

Accesspoints::Accesspoints() {
  list = new LinkedList<AP>;
}

void Accesspoints::sort() {
  list->sort([](AP & a, AP & b) -> int{
    if (WiFi.RSSI(a.id) == WiFi.RSSI(b.id)) return 0;
    if (WiFi.RSSI(a.id) > WiFi.RSSI(b.id)) return -1;
    if (WiFi.RSSI(a.id) < WiFi.RSSI(b.id)) return 1;
  });
}

void Accesspoints::sortAfterChannel() {
  list->sort([](AP & a, AP & b) -> int{
    if (WiFi.channel(a.id) == WiFi.channel(b.id)) return 0;
    if (WiFi.channel(a.id) < WiFi.channel(b.id)) return -1;
    if (WiFi.channel(a.id) > WiFi.channel(b.id)) return 1;
  });
  changed = true;
}

void Accesspoints::add(uint8_t id, bool selected) {
  list->add(AP{id, selected});
  changed = true;
}

void Accesspoints::printAll() {
  prntln(AP_HEADER);
  if (list->size() == 0)
    prntln(AP_LIST_EMPTY);
  else
    for (uint8_t i = 0; i < list->size(); i++)
      print(i, i == 0, i == list->size() - 1);
}

void Accesspoints::printSelected() {
  prntln(AP_HEADER);
  uint8_t max = selected();
  if (selected() == 0) {
    prntln(AP_NO_AP_SELECTED);
    return;
  }
  
  for (uint8_t i = 0, j = 0; i < list->size(), j < max; i++) {
    if (getSelected(i)) {
      print(i, j == 0, j == max - 1);
      j++;
    }
  }
}

void Accesspoints::print(uint8_t num) {
  print(num, true, true);
}

void Accesspoints::print(uint8_t num, bool header, bool footer) {
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

String Accesspoints::getSSID(uint8_t num) {
  if (!check(num)) return String();
  if (getHidden(num)){
    return str(AP_HIDDEN);
  } else {
    String ssid = WiFi.SSID(list->get(num).id);
    ssid = ssid.substring(0,32);
    ssid = fixUtf8(ssid);
    return ssid;
  }
}

String Accesspoints::getNameStr(uint8_t num) {
  if (!check(num)) return String();
  return names.find(getMac(num));
}

uint8_t Accesspoints::getCh(uint8_t num) {
  if (!check(num)) return 0;
  return WiFi.channel(list->get(num).id);
}

int8_t Accesspoints::getRSSI(uint8_t num) {
  if (!check(num)) return 0;
  return WiFi.RSSI(list->get(num).id);
}

uint8_t Accesspoints::getEnc(uint8_t num) {
  if (!check(num)) return 0;
  return WiFi.encryptionType(list->get(num).id);
}

String Accesspoints::getEncStr(uint8_t num) {
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

String Accesspoints::getSelectedStr(uint8_t num) {
  return b2a(getSelected(num));
}

uint8_t* Accesspoints::getMac(uint8_t num) {
  if (!check(num)) return 0;
  return WiFi.BSSID(list->get(num).id);
}

String Accesspoints::getMacStr(uint8_t num) {
  if (!check(num)) return String();
  uint8_t* mac = getMac(num);
  String value;
  for (uint8_t i = 0; i < 6; i++) {
    if (mac[i] < 0x10) value += ZERO;
    value += String(mac[i], HEX);
    if (i < 5) value += DOUBLEPOINT;
  }
  return value;
}

String Accesspoints::getVendorStr(uint8_t num) {
  if (!check(num)) return String();
  return searchVendor(getMac(num));
}

bool Accesspoints::getHidden(uint8_t num) {
  if (!check(num)) return false;
  return WiFi.isHidden(list->get(num).id);
}

bool Accesspoints::getSelected(uint8_t num) {
  if (!check(num)) return false;
  return list->get(num).selected;
}

void Accesspoints::select(uint8_t num) {
  if (!check(num)) return;
  AP changedAP = list->get(num);
  changedAP.selected = true;
  list->set(num, changedAP);
  prnt(AP_SELECTED);
  prntln(getSSID(num));
  changed = true;
}

void Accesspoints::deselect(uint8_t num) {
  if (!check(num)) return;
  AP changedAP = list->get(num);
  changedAP.selected = false;
  list->set(num, changedAP);
  prnt(AP_DESELECTED);
  prntln(getSSID(num));
  changed = true;
}

void Accesspoints::remove(uint8_t num) {
  prnt(AP_REMOVED);
  prntln(getSSID(num));
  list->remove(num);
  changed = true;
}

void Accesspoints::selectAll() {
  for (uint8_t i = 0; i < count(); i++)
    internal_select(i);
  prntln(AP_SELECTED_ALL);
  changed = true;
}

void Accesspoints::deselectAll() {
  for (uint8_t i = 0; i < count(); i++)
    internal_deselect(i);
  prntln(AP_DESELECTED_ALL);
  changed = true;
}

void Accesspoints::removeAll() {
  list->clear();
  prntln(AP_REMOVED_ALL);
  changed = true;
}

uint8_t Accesspoints::count() {
  return (uint8_t)list->size();
}

uint8_t Accesspoints::selected() {
  uint8_t num = 0;
  for (uint8_t i = 0; i < count(); i++)
    if (getSelected(i)) num++;
  return num;
}

bool Accesspoints::check(uint8_t num) {
  if (internal_check(num)) return true;
  prnt(AP_NO_AP_ERROR);
  prntln((String)num);
  return false;
}

bool Accesspoints::internal_check(uint8_t num) {
  return num >= 0 && num < count();
}

void Accesspoints::internal_select(uint8_t num) {
  AP changedAP = list->get(num);
  changedAP.selected = true;
  list->set(num, changedAP);
}

void Accesspoints::internal_deselect(uint8_t num) {
  AP changedAP = list->get(num);
  changedAP.selected = false;
  list->set(num, changedAP);
}

void Accesspoints::internal_remove(uint8_t num) {
  list->remove(num);
}


