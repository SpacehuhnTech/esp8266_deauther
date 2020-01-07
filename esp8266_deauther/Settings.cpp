#include "Settings.h"

#include <Hash.h>         // sha1() used in calcHash()
#include "EEPROMHelper.h" // To load and save settings_t

// ===== INTERNAL ===== //
bool operator==(settings_hash_t a, settings_hash_t b) {
    for (int i = 0; i<20; i++)
        if (a.hash[i] != b.hash[i]) return false;
    return true;
}

bool operator==(version_t a, version_t b) {
    return a.major == b.major && a.minor == b.minor && a.revision == b.revision;
}

void jsonStr(String& str, const char* name, const char* value) {
    str += '"';
    str += String(name);
    str += '"';
    str += ':';
    str += '"';
    str += String(value);
    str += '"';
    str += ',';
}

void jsonFlag(String& str, const char* name, bool value) {
    str += '"';
    str += String(name);
    str += '"';
    str += ':';
    str += value ? String(S_JSON_TRUE) : String(S_JSON_FALSE);
    str += ',';
}

void jsonValue(String& str, const char* name, int value) {
    str += '"';
    str += String(name);
    str += '"';
    str += ':';
    str += String(value);
    str += ',';
}

void jsonHex(String& str, const char* name, uint8_t* byteArr, int len) {
    str += '"';
    str += String(name);
    str += '"';
    str += ':';

    str += '"';

    for (int i = 0; i<len; i++) {
        if (i > 0) str += ':';
        if (byteArr[i] < 0x10) str += '0';
        str += String(byteArr[i], HEX);
    }

    str += '"';
    str += ',';
}

void jsonDec(String& str, const char* name, uint8_t* byteArr, int len) {
    str += '"';
    str += String(name);
    str += '"';
    str += ':';

    str += '"';

    for (int i = 0; i<len; i++) {
        if (i > 0) str += '.';
        str += String(byteArr[i]);
    }

    str += '"';
    str += ',';
}

// ========== PRIVATE ========== //
settings_hash_t Settings::calcHash(settings_t data) {
    settings_hash_t hash;

    sha1((uint8_t*)&data, sizeof(settings_t), &hash.hash[0]);
    return hash;
}

String Settings::getJsonStr() {
    String str((char*)0);

    str.reserve(600);

    str += '{';

    // Version
    jsonStr(str, S_JSON_VERSION, DEAUTHER_VERSION);

    // Autosave
    jsonFlag(str, S_JSON_AUTOSAVE, data.autosave.enabled);
    jsonValue(str, S_JSON_AUTOSAVETIME, data.autosave.time);

    // Attack
    jsonFlag(str, S_JSON_BEACONCHANNEL, data.attack.attack_all_ch);
    jsonFlag(str, S_JSON_RANDOMTX, data.attack.random_tx);
    jsonValue(str, S_JSON_ATTACKTIMEOUT, data.attack.timeout);
    jsonValue(str, S_JSON_DEAUTHSPERTARGET, data.attack.deauths_per_target);
    jsonValue(str, S_JSON_DEAUTHREASON, data.attack.deauth_reason);
    jsonFlag(str, S_JSON_BEACONINTERVAL, data.attack.beacon_interval == INTERVAL_1S);
    jsonValue(str, S_JSON_PROBESPERSSID, data.attack.probe_frames_per_ssid);

    // WiFi
    jsonValue(str, S_JSON_CHANNEL, data.wifi.channel);
    jsonHex(str, S_JSON_MACST, data.wifi.mac_st, 6);
    jsonHex(str, S_JSON_MACAP, data.wifi.mac_ap, 6);

    // Sniffer
    jsonValue(str, S_JSON_CHTIME, data.sniffer.channel_time);
    jsonValue(str, S_JSON_MIN_DEAUTHS, data.sniffer.min_deauth_frames);

    // Access Point
    jsonStr(str, S_JSON_SSID, data.ap.ssid);
    jsonStr(str, S_JSON_PASSWORD, data.ap.password);
    jsonFlag(str, S_JSON_HIDDEN, data.ap.hidden);
    jsonDec(str, S_JSON_IP, data.ap.ip, 4);

    // Web Interface
    jsonFlag(str, S_JSON_WEBINTERFACE, data.web.enabled);
    jsonFlag(str, S_JSON_CAPTIVEPORTAL, data.web.captive_portal);
    jsonFlag(str, S_JSON_WEB_SPIFFS, data.web.use_spiffs);
    jsonStr(str, S_JSON_LANG, data.web.lang);

    // CLI
    jsonFlag(str, S_JSON_SERIALINTERFACE, data.cli.enabled);
    jsonFlag(str, S_JSON_SERIAL_ECHO, data.cli.serial_echo);

    // LED
    jsonFlag(str, S_JSON_LEDENABLED, data.led.enabled);

    // Display
    jsonFlag(str, S_JSON_DISPLAYINTERFACE, data.display.enabled);
    jsonValue(str, S_JSON_DISPLAY_TIMEOUT, data.display.timeout);

    str[str.length()-1] = '}';

    return str;
}

// ========== PUBLIC ========== //
void Settings::load() {
    prnt(S_SETTINGS_LOADED);

    // read hash from eeprom
    settings_hash_t hash;

    EEPROMHelper::getObject(SETTINGS_HASH_ADDR, hash);

    // read data from eeproms
    settings_t newData;
    EEPROMHelper::getObject(SETTINGS_ADDR, newData);

    // calc and check hash
    if ((newData.version == data.version) && (calcHash(newData) == hash)) {
        this->data = newData;
        prntln(S_OK);
    } else {
        prntln(S_INVALID_HASH);
    }

    // check and fix mac
    if (!macValid(data.wifi.mac_st)) getRandomMac(data.wifi.mac_st);
    if (!macValid(data.wifi.mac_ap)) getRandomMac(data.wifi.mac_ap);

    changed = true;
}

void Settings::reset() {
    settings_t newData;

    this->data = newData;

    prntln(S_SETTINGS_RESETED);
}

void Settings::save(bool force) {
    if (force || changed) {
        EEPROMHelper::saveObject(SETTINGS_HASH_ADDR, calcHash(data));
        EEPROMHelper::saveObject(SETTINGS_ADDR, data);

        changed = false;

        String buf = getJsonStr();
        if (writeFile(SETTINGS_PATH, buf)) {
            prnt(S_SETTINGS_SAVED);
        } else {
            prnt(S_ERROR_SAVING);
        }
        prntln(SETTINGS_PATH);
    }
}

void Settings::print() {
    String settingsJson = getJsonStr();

    settingsJson.replace("\":", " = ");
    settingsJson.replace("= 0\r\n", "= false\r\n");
    settingsJson.replace("= 1\r\n", "= true\r\n");
    settingsJson.replace("\"", "");
    settingsJson.replace("{", "");
    settingsJson.replace("}", "");
    settingsJson.replace(",", "\r\n");

    prntln(S_SETTINGS_HEADER);
    prntln(settingsJson);
}

// ===== GETTERS ===== //

const settings_t& Settings::getAllSettings() {
    return data;
}

const version_t& Settings::getVersion() {
    return data.version;
}

const autosave_settings_t& Settings::getAutosaveSettings() {
    return data.autosave;
}

const attack_settings_t& Settings::getAttackSettings() {
    return data.attack;
}

const wifi_settings_t& Settings::getWifiSettings() {
    return data.wifi;
}

const sniffer_settings_t& Settings::getSnifferSettings() {
    return data.sniffer;
}

const access_point_settings_t& Settings::getAccessPointSettings() {
    return data.ap;
}

const web_settings_t& Settings::getWebSettings() {
    return data.web;
}

const cli_settings_t& Settings::getCLISettings() {
    return data.cli;
}

const led_settings_t& Settings::getLEDSettings() {
    return data.led;
}

const display_settings_t& Settings::getDisplaySettings() {
    return data.display;
}

// ===== SETTERS ===== //

void Settings::setAllSettings(settings_t& newSettings) {
    newSettings.version = this->data.version;
    data                = newSettings;
    changed             = true;
}

void Settings::setAutosaveSettings(const autosave_settings_t& autosave) {
    data.autosave = autosave;
    changed       = true;
}

void Settings::setAttackSettings(const attack_settings_t& attack) {
    data.attack = attack;
    changed     = true;
}

void Settings::setWifiSettings(const wifi_settings_t& wifi) {
    data.wifi = wifi;
    changed   = true;
}

void Settings::setSnifferSettings(const sniffer_settings_t& sniffer) {
    data.sniffer = sniffer;
    changed      = true;
}

void Settings::setAccessPointSettings(const access_point_settings_t& ap) {
    data.ap = ap;
    changed = true;
}

void Settings::setWebSettings(const web_settings_t& web) {
    data.web = web;
    changed  = true;
}

void Settings::setCLISettings(const cli_settings_t& cli) {
    data.cli = cli;
    changed  = true;
}

void Settings::setLEDSettings(const led_settings_t& led) {
    data.led = led;
    changed  = true;
}

void Settings::setDisplaySettings(const display_settings_t& display) {
    data.display = display;
    changed      = true;
}