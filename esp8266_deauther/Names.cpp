#include "Names.h"

Names::Names() {
    list = new SimpleList<Device>;
}

void Names::load() {
    internal_removeAll();

    DynamicJsonBuffer jsonBuffer(4000);

    checkFile(FILE_PATH, String(OPEN_BRACKET) + String(CLOSE_BRACKET));
    JsonArray& arr = parseJSONFile(FILE_PATH, jsonBuffer);

    for (uint32_t i = 0; i < arr.size() && i < NAME_LIST_SIZE; i++) {
        JsonArray& tmpArray = arr.get<JsonVariant>(i);
        internal_add(tmpArray.get<String>(0), tmpArray.get<String>(2), tmpArray.get<String>(3), tmpArray.get<uint8_t>(
                         4), false);
        sort();
    }

    prnt(N_LOADED);
    prntln(FILE_PATH);
}

void Names::load(String filepath) {
    String tmp = FILE_PATH;

    FILE_PATH = filepath;
    load();
    FILE_PATH = tmp;
}

void Names::save(bool force) {
    if (!force && !changed) {
        return;

        prntln(N_SAVED);
    }

    String buf = String(OPEN_BRACKET); // [

    if (!writeFile(FILE_PATH, buf)) {
        prnt(F_ERROR_SAVING);
        prntln(FILE_PATH);
        return;
    }

    buf = String();

    String name;
    int    c = count();

    for (int i = 0; i < c; i++) {
        name = escape(getName(i));

        buf += String(OPEN_BRACKET) + String(DOUBLEQUOTES) + getMacStr(i) + String(DOUBLEQUOTES) + String(COMMA); // ["00:11:22:00:11:22",
        buf += String(DOUBLEQUOTES) + getVendorStr(i) + String(DOUBLEQUOTES) + String(COMMA);                     // "vendor",
        buf += String(DOUBLEQUOTES) + name + String(DOUBLEQUOTES) + String(COMMA);                                // "name",
        buf += String(DOUBLEQUOTES) + getBssidStr(i) + String(DOUBLEQUOTES) + String(COMMA);                      // "00:11:22:00:11:22",
        buf += String(getCh(i)) + String(COMMA);                                                                  // 1,
        buf += b2s(getSelected(i)) + String(CLOSE_BRACKET);                                                       // false]

        if (i < c - 1) buf += COMMA;                                                                              // ,

        if (buf.length() >= 1024) {
            if (!appendFile(FILE_PATH, buf)) {
                prnt(F_ERROR_SAVING);
                prntln(FILE_PATH);
                return;
            }

            buf = String();
        }
    }

    buf += String(CLOSE_BRACKET); // ]

    if (!appendFile(FILE_PATH, buf)) {
        prnt(F_ERROR_SAVING);
        prntln(FILE_PATH);
        return;
    }

    prnt(N_SAVED);
    prntln(FILE_PATH);
    changed = false;
}

void Names::save(bool force, String filepath) {
    String tmp = FILE_PATH;

    FILE_PATH = filepath;
    save(force);
    FILE_PATH = tmp;
}

void Names::sort() {
    list->setCompare([](Device& a, Device& b) -> int {
        return memcmp(a.mac, b.mac, 6);
    });
    list->sort();
}

void Names::removeAll() {
    internal_removeAll();
    prntln(N_REMOVED_ALL);
    changed = true;
}

bool Names::check(int num) {
    if (internal_check(num)) return true;

    prnt(N_ERROR_NOT_FOUND);
    prntln(num);
    return false;
}

int Names::findID(uint8_t* mac) {
    for (int i = 0; i < list->size(); i++) {
        if (memcmp(mac, list->get(i).mac, 6) == 0) return i;
    }

    return -1;
}

String Names::find(uint8_t* mac) {
    int num = findID(mac);

    if (num >= 0) return getName(num);
    else return String();
}

void Names::print(int num) {
    print(num, true, true);
}

void Names::print(int num, bool header, bool footer) {
    if (!check(num)) return;

    if (header) {
        prntln(N_TABLE_HEADER);
        prntln(N_TABLE_DIVIDER);
    }

    prnt(leftRight(String(), (String)num, 2));
    prnt(leftRight(String(SPACE) + getMacStr(num), String(), 18));
    prnt(leftRight(String(SPACE) + getVendorStr(num), String(), 9));
    prnt(leftRight(String(SPACE) + getName(num), String(), 17));
    prnt(leftRight(String(SPACE) + getBssidStr(num), String(), 18));
    prnt(leftRight(String(SPACE), (String)getCh(num), 3));
    prntln(leftRight(String(SPACE) + getSelectedStr(num), String(), 9));

    if (footer) prntln(N_TABLE_DIVIDER);
}

void Names::printAll() {
    prntln(N_HEADER);
    int c = count();

    if (c == 0) prntln(N_ERROR_LIST_EMPTY);
    else
        for (int i = 0; i < c; i++) print(i, i == 0, i == c - 1);
}

void Names::printSelected() {
    prntln(N_TABLE_HEADER);
    int max = selected();
    int c   = count();

    if (max == 0) {
        prntln(N_ERROR_NO_SELECTED);
        return;
    }

    for (int i = 0, j = 0; i < c && j < max; i++) {
        if (getSelected(i)) {
            print(i, j == 0, j == max - 1);
            j++;
        }
    }
}

void Names::add(uint8_t* mac, String name, uint8_t* bssid, uint8_t ch, bool selected, bool force) {
    if (count() >= NAME_LIST_SIZE) {
        if (force) internal_remove(0);
        else {
            prntln(N_ERROR_LIST_FULL);
            return;
        }
    }

    if (name.length() > NAME_MAX_LENGTH) name = name.substring(0, NAME_MAX_LENGTH);

    internal_add(mac, name, bssid, ch, selected);
    sort();

    prnt(N_ADDED);
    prntln(name);
    changed = true;
}

void Names::add(String macStr, String name, String bssidStr, uint8_t ch, bool selected, bool force) {
    if (count() >= NAME_LIST_SIZE) {
        if (force) internal_remove(0);
        else {
            prntln(N_ERROR_LIST_FULL);
            return;
        }
    }

    if (name.length() > NAME_MAX_LENGTH) name = name.substring(0, NAME_MAX_LENGTH);

    internal_add(macStr, name, bssidStr, ch, selected);
    sort();

    prnt(N_ADDED);
    prntln(name);
    changed = true;
}

void Names::replace(int num, String macStr, String name, String bssidStr, uint8_t ch, bool selected) {
    if (!check(num)) return;

    remove(num);

    internal_add(macStr, name, bssidStr, ch, selected);
    sort();
    prnt(N_REPLACED);
    prntln(name);
    changed = true;
}

void Names::remove(int num) {
    if (!check(num)) return;

    prnt(N_REMOVED);
    prntln(getName(num));
    internal_remove(num);
    changed = true;
}

void Names::setName(int num, String name) {
    if (!check(num)) return;

    internal_add(getMac(num), name, getBssid(num), getCh(num), getSelected(num));

    prntln(N_CHANGED_NAME);

    internal_remove(num);
    sort();
    changed = true;
}

void Names::setMac(int num, String macStr) {
    if (!check(num)) return;

    uint8_t mac[6];
    strToMac(macStr, mac);
    internal_add(mac, getName(num), getBssid(num), getCh(num), getSelected(num));
    prntln(N_CHANGED_MAC);
    internal_remove(num);
    sort();
    changed = true;
}

void Names::setCh(int num, uint8_t ch) {
    if (!check(num)) return;

    internal_add(getMac(num), getName(num), getBssid(num), ch, getSelected(num));
    prntln(N_CHANGED_CH);
    internal_remove(num);
    sort();
    changed = true;
}

void Names::setBSSID(int num, String bssidStr) {
    if (!check(num)) return;

    uint8_t mac[6];
    strToMac(bssidStr, mac);
    internal_add(getMac(num), getName(num), mac, getCh(num), getSelected(num));
    prntln(N_CHANGED_BSSID);
    internal_remove(num);
    sort();
    changed = true;
}

void Names::select(int num) {
    if (!check(num)) return;

    internal_select(num);
    prnt(N_SELECTED);
    prntln(getName(num));
    changed = true;
}

void Names::select(String name) {
    int c = count();

    for (int i = 0; i < c; i++) {
        if (getName(i).equals(name)) {
            select(i);
            return;
        }
    }
    prnt(N_ERROR_NOT_FOUND);
    prntln(name);
}

void Names::deselect(int num) {
    if (!check(num)) return;

    internal_deselect(num);
    prnt(N_DESELECTED);
    prntln(getName(num));
    changed = true;
}

void Names::deselect(String name) {
    int c = count();

    for (int i = 0; i < c; i++) {
        if (getName(i).equals(name)) {
            deselect(i);
            return;
        }
    }
    prnt(N_ERROR_NOT_FOUND);
    prnt(name);
}

void Names::selectAll() {
    int c = count();

    for (int i = 0; i < c; i++) internal_select(i);
    prntln(N_SELECTED_ALL);
}

void Names::deselectAll() {
    int c = count();

    for (int i = 0; i < c; i++) internal_deselect(i);
    prntln(N_DESELECTED_ALL);
}

uint8_t* Names::getMac(int num) {
    if (!check(num)) return NULL;

    return list->get(num).mac;
}

uint8_t* Names::getBssid(int num) {
    if (!check(num)) return NULL;

    return list->get(num).apBssid;
}

String Names::getMacStr(int num) {
    if (!check(num)) return String();

    uint8_t* mac = getMac(num);
    return bytesToStr(mac, 6);
}

String Names::getVendorStr(int num) {
    if (!check(num)) return String();

    return searchVendor(list->get(num).mac);
}

String Names::getBssidStr(int num) {
    String value;

    if (getBssid(num) != NULL) {
        uint8_t* mac = getBssid(num);

        for (int i = 0; i < 6; i++) {
            if (mac[i] < 0x10) value += ZERO;
            value += String(mac[i], HEX);

            if (i < 5) value += DOUBLEPOINT;
        }
    }
    return value;
}

String Names::getName(int num) {
    if (!check(num)) return String();

    return String(list->get(num).name);
}

String Names::getSelectedStr(int num) {
    return b2a(getSelected(num));
}

uint8_t Names::getCh(int num) {
    if (!check(num)) return 1;

    return list->get(num).ch;
}

bool Names::getSelected(int num) {
    if (!check(num)) return false;

    return list->get(num).selected;
}

bool Names::isStation(int num) {
    return getBssid(num) != NULL;
}

int Names::count() {
    return list->size();
}

int Names::selected() {
    int num = 0;

    for (int i = 0; i < count(); i++)
        if (getSelected(i)) num++;
    return num;
}

int Names::stations() {
    int num = 0;

    for (int i = 0; i < count(); i++)
        if (isStation(i)) num++;
    return num;
}

bool Names::internal_check(int num) {
    return num >= 0 && num < count();
}

void Names::internal_select(int num) {
    Device newDevice = list->get(num);

    newDevice.selected = true;
    list->replace(num, newDevice);
}

void Names::internal_deselect(int num) {
    Device newDevice = list->get(num);

    newDevice.selected = false;
    list->replace(num, newDevice);
}

void Names::internal_add(uint8_t* mac, String name, uint8_t* bssid, uint8_t ch, bool selected) {
    uint8_t* deviceMac = (uint8_t*)malloc(6);

    if (name.length() > NAME_MAX_LENGTH) name = name.substring(0, NAME_MAX_LENGTH);
    char* deviceName     = (char*)malloc(name.length() + 1);
    uint8_t* deviceBssid = NULL;

    name = fixUtf8(name);

    memcpy(deviceMac, mac, 6);
    strcpy(deviceName, name.c_str());

    if (bssid) {
        deviceBssid = (uint8_t*)malloc(6);
        memcpy(deviceBssid, bssid, 6);
    }

    if ((ch < 1) || (ch > 14)) ch = 1;

    Device newDevice;
    newDevice.mac      = deviceMac;
    newDevice.name     = deviceName;
    newDevice.apBssid  = deviceBssid;
    newDevice.ch       = ch;
    newDevice.selected = selected;

    list->add(newDevice);
}

void Names::internal_add(String macStr, String name, String bssidStr, uint8_t ch, bool selected) {
    uint8_t mac[6];

    if (!strToMac(macStr, mac)) return;

    if (bssidStr.length() == 17) {
        uint8_t bssid[6];
        strToMac(bssidStr, bssid);
        internal_add(mac, name, bssid, ch, selected);
    } else {
        internal_add(mac, name, NULL, ch, selected);
    }
}

void Names::internal_remove(int num) {
    free(list->get(num).mac);
    free(list->get(num).name);

    if (list->get(num).apBssid) free(list->get(num).apBssid);
    list->remove(num);
}

void Names::internal_removeAll() {
    while (count() > 0) {
        free(list->get(0).mac);
        free(list->get(0).name);

        if (list->get(0).apBssid) free(list->get(0).apBssid);
        list->remove(0);
    }
}