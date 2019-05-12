#include "Accesspoints.h"

Accesspoints::Accesspoints() {
    list = new SimpleList<AP>;
}

void Accesspoints::sort() {
    list->setCompare([](AP& a, AP& b) -> int {
        if (WiFi.RSSI(a.id) > WiFi.RSSI(b.id)) return -1;

        if (WiFi.RSSI(a.id) == WiFi.RSSI(b.id)) return 0;

        return 1;
    });
    list->sort();
    changed = true;
}

void Accesspoints::sortAfterChannel() {
    list->setCompare([](AP& a, AP& b) -> int {
        if (WiFi.channel(a.id) < WiFi.channel(b.id)) return -1;

        if (WiFi.channel(a.id) == WiFi.channel(b.id)) return 0;

        return 1;
    });
    list->sort();
    changed = true;
}

void Accesspoints::add(uint8_t id, bool selected) {
    list->add(AP{ id, selected });
    changed = true;
}

void Accesspoints::printAll() {
    prntln(AP_HEADER);
    int c = count();

    if (c == 0) prntln(AP_LIST_EMPTY);
    else
        for (int i = 0; i < c; i++) print(i, i == 0, i == c - 1);
}

void Accesspoints::printSelected() {
    prntln(AP_HEADER);
    int max = selected();

    if (selected() == 0) {
        prntln(AP_NO_AP_SELECTED);
        return;
    }
    int c = count();
    int j = 0;

    for (int i = 0; i < c && j < max; i++) {
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
    prnt(leftRight(String(), (String)num, 2));
    prnt(leftRight(String(SPACE) + getSSID(num), String(), 33));
    prnt(leftRight(String(SPACE) + getNameStr(num), String(), 17));
    prnt(leftRight(String(SPACE), (String)getCh(num), 3));
    prnt(leftRight(String(SPACE), (String)getRSSI(num), 5));
    prnt(leftRight(String(SPACE), getEncStr(num), 5));
    prnt(leftRight(String(SPACE) + getMacStr(num), String(), 18));
    prnt(leftRight(String(SPACE) + getVendorStr(num), String(), 9));
    prntln(leftRight(String(SPACE) + getSelectedStr(num), String(), 9));

    if (footer) {
        prntln(AP_TABLE_DIVIDER);
    }
}

String Accesspoints::getSSID(int num) {
    if (!check(num)) return String();

    if (getHidden(num)) {
        return str(AP_HIDDE_SSID);
    } else {
        String ssid = WiFi.SSID(getID(num));
        ssid = ssid.substring(0, 32);
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
    return bytesToStr(mac, 6);
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

    return list->get(num).selected;
}

uint8_t Accesspoints::getID(int num) {
    if (!check(num)) return -1;

    return list->get(num).id;
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

void Accesspoints::select(String ssid) {
    for (int i = 0; i < list->size(); i++) {
        if (getSSID(i).equalsIgnoreCase(ssid)) select(i);
    }
}

void Accesspoints::deselect(String ssid) {
    for (int i = 0; i < list->size(); i++) {
        if (getSSID(i).equalsIgnoreCase(ssid)) deselect(i);
    }
}

void Accesspoints::remove(String ssid) {
    for (int i = 0; i < list->size(); i++) {
        if (getSSID(i).equalsIgnoreCase(ssid)) remove(i);
    }
}

void Accesspoints::selectAll() {
    for (int i = 0; i < count(); i++) list->replace(i, AP{ list->get(i).id, true });
    prntln(AP_SELECTED_ALL);
    changed = true;
}

void Accesspoints::deselectAll() {
    for (int i = 0; i < count(); i++) list->replace(i, AP{ list->get(i).id, false });
    prntln(AP_DESELECTED_ALL);
    changed = true;
}

void Accesspoints::removeAll() {
    while (count() > 0) internal_remove(0);
    prntln(AP_REMOVED_ALL);
    changed = true;
}

int Accesspoints::find(uint8_t id) {
    int s = list->size();

    for (int i = 0; i < s; i++) {
        if (list->get(i).id == id) return i;
    }
    return -1;
}

int Accesspoints::count() {
    return list->size();
}

int Accesspoints::selected() {
    int c = 0;

    for (int i = 0; i < list->size(); i++) c += list->get(i).selected;
    return c;
}

bool Accesspoints::check(int num) {
    if (internal_check(num)) return true;

    prnt(AP_NO_AP_ERROR);
    prntln((String)num);
    return false;
}

bool Accesspoints::internal_check(int num) {
    return num >= 0 && num < count();
}

void Accesspoints::internal_select(int num) {
    list->replace(num, AP{ list->get(num).id, true });
}

void Accesspoints::internal_deselect(int num) {
    list->replace(num, AP{ list->get(num).id, false });
}

void Accesspoints::internal_remove(int num) {
    list->remove(num);
}