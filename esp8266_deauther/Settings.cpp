#include "Settings.h"

#include <Hash.h>         // sha1() used in calcHash()
#include "EEPROMHelper.h" // To load and save settings_t

bool operator==(settings_hash_t a, settings_hash_t b) {
    for (int i = 0; i<20; i++)
        if (a.hash[i] != b.hash[i]) return false;
    return true;
}

bool operator==(version_t a, version_t b) {
    return a.major == b.major && a.minor == b.minor && a.revision == b.revision;
}

// ========== PRIVATE ========== //
settings_hash_t Settings::calcHash(settings_t data) {
    settings_hash_t hash;

    sha1((uint8_t*)&data, sizeof(settings_t), &hash.hash[0]);
    return hash;
}

String Settings::getJsonStr() {
    DynamicJsonBuffer jsonBuffer(4000);
    JsonObject& data = jsonBuffer.createObject();

    // Version
    data.set("version", DEAUTHER_VERSION);

    // AP
    data.set(keyword(S_SSID), getSSID());
    data.set(keyword(S_PASSWORD), getPassword());
    data.set(keyword(S_CHANNEL), getChannel());
    data.set(keyword(S_HIDDEN), getHidden());
    data.set(keyword(S_CAPTIVEPORTAL), getCaptivePortal());

    // GENERAL
    data.set(keyword(S_LANG), getLang());
    data.set(keyword(S_AUTOSAVE), getAutosave());
    data.set(keyword(S_AUTOSAVETIME), getAutosaveTime());
    data.set(keyword(S_DISPLAYINTERFACE), getDisplayInterface());
    data.set(keyword(S_DISPLAY_TIMEOUT), getDisplayTimeout());
    data.set(keyword(S_SERIALINTERFACE), getCLI());
    data.set(keyword(S_SERIAL_ECHO), getSerialEcho());
    data.set(keyword(S_WEBINTERFACE), getWebInterface());
    data.set(keyword(S_WEB_SPIFFS), getWebSpiffs());
    data.set(keyword(S_LEDENABLED), getLedEnabled());
    data.set(keyword(S_MACAP), macToStr(getMacAP()));
    data.set(keyword(S_MACST), macToStr(getMacSt()));

    // SCAN
    data.set(keyword(S_CHTIME), getChTime());
    data.set(keyword(S_MIN_DEAUTHS), getMinDeauths());

    // ATTACK
    data.set(keyword(S_ATTACKTIMEOUT), getAttackTimeout());
    data.set(keyword(S_DEAUTHSPERTARGET), getDeauthsPerTarget());
    data.set(keyword(S_DEAUTHREASON), getDeauthReason());
    data.set(keyword(S_BEACONCHANNEL), getBeaconChannel());
    data.set(keyword(S_BEACONINTERVAL), getBeaconInterval());
    data.set(keyword(S_RANDOMTX), getRandomTX());
    data.set(keyword(S_PROBESPERSSID), getProbesPerSSID());

    String buf;
    data.printTo(buf);

    return buf;
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
    if (!macValid(getMacSt())) getRandomMac(data.wifi.mac_st);
    if (!macValid(getMacAP())) getRandomMac(data.wifi.mac_ap);

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
            prntln(SETTINGS_PATH);
        } else {
            prnt(F("ERROR: saving "));
            prntln(SETTINGS_PATH);
        }
    }
}

void Settings::print() {
    String settingsJson = getJsonStr();

    settingsJson.replace("{", "{\r\n");
    settingsJson.replace("}", "\r\n}");
    settingsJson.replace(",", "\r\n");

    prntln(S_SETTINGS_HEADER);
    prntln(settingsJson);
}

void Settings::set(const char* str, String value) {
    // booleans
    if (eqls(str, S_BEACONCHANNEL)) setBeaconChannel(s2b(value));
    else if (eqls(str, S_AUTOSAVE)) setAutosave(s2b(value));
    else if (eqls(str, S_BEACONINTERVAL)) setBeaconInterval(s2b(value));
    else if (eqls(str, S_SERIALINTERFACE)) setCLI(s2b(value));
    else if (eqls(str, S_DISPLAYINTERFACE)) setDisplayInterface(s2b(value));
    else if (eqls(str, S_WEBINTERFACE)) setWebInterface(s2b(value));
    else if (eqls(str, S_RANDOMTX)) setRandomTX(s2b(value));
    else if (eqls(str, S_LEDENABLED)) setLedEnabled(s2b(value));
    else if (eqls(str, S_HIDDEN)) setHidden(s2b(value));
    else if (eqls(str, S_CAPTIVEPORTAL)) setCaptivePortal(s2b(value));
    else if (eqls(str, S_SERIAL_ECHO)) setSerialEcho(s2b(value));
    else if (eqls(str, S_WEB_SPIFFS)) setWebSpiffs(s2b(value));

    // integer
    else if (eqls(str, S_AUTOSAVETIME)) setAutosaveTime(value.toInt());
    else if (eqls(str, S_DEAUTHSPERTARGET)) setDeauthsPerTarget(value.toInt());
    else if (eqls(str, S_CHTIME)) setChTime(value.toInt());
    else if (eqls(str, S_CHANNEL)) setChannel(value.toInt());
    else if (eqls(str, S_DEAUTHREASON)) setDeauthReason(value.toInt());
    else if (eqls(str, S_ATTACKTIMEOUT)) setAttackTimeout(value.toInt());
    else if (eqls(str, S_PROBESPERSSID)) setProbesPerSSID(value.toInt());
    else if (eqls(str, S_MIN_DEAUTHS)) setMinDeauths(value.toInt());
    else if (eqls(str, S_DISPLAY_TIMEOUT)) setDisplayTimeout(value.toInt());

    // strings
    else if (eqls(str, S_LANG)) setLang(value);
    else if (eqls(str, S_SSID)) setSSID(value);
    else if (eqls(str, S_PASSWORD)) setPassword(value);
    else if (eqls(str, S_MACAP)) setMacAP(value);
    else if (eqls(str, S_MACST)) setMacSt(value);
    else if (eqls(str, S_MAC) && value.equalsIgnoreCase("random")) {
        setMacSt(value);
        setMacAP(value);
    }

    else if (eqls(str, S_VERSION)) prntln(S_ERROR_VERSION);

    else {
        prnt(S_ERROR_NOT_FOUND);
        prntln(str);
        return;
    }

    prnt(S_CHANGED_SETTING);
    prntln(str);
}

String Settings::get(const char* str) {
    if (eqls(str, S_SETTINGS)) print();
    // booleans
    else if (eqls(str, S_BEACONCHANNEL)) return b2s(getBeaconChannel());
    else if (eqls(str, S_BEACONINTERVAL)) return b2s(getBeaconInterval());
    else if (eqls(str, S_SERIALINTERFACE)) return b2s(getCLI());
    else if (eqls(str, S_DISPLAYINTERFACE)) return b2s(getDisplayInterface());
    else if (eqls(str, S_WEBINTERFACE)) return b2s(getWebInterface());
    else if (eqls(str, S_RANDOMTX)) return b2s(getRandomTX());
    else if (eqls(str, S_LEDENABLED)) return b2s(getLedEnabled());
    else if (eqls(str, S_HIDDEN)) return b2s(getHidden());
    else if (eqls(str, S_CAPTIVEPORTAL)) return b2s(getCaptivePortal());
    else if (eqls(str, S_SERIAL_ECHO)) return b2s(getSerialEcho());
    else if (eqls(str, S_WEB_SPIFFS)) return b2s(getWebSpiffs());

    // integer
    else if (eqls(str, S_DEAUTHSPERTARGET)) return (String)getDeauthsPerTarget();
    else if (eqls(str, S_CHTIME)) return (String)getChTime();
    else if (eqls(str, S_ATTACKTIMEOUT)) return (String)getAttackTimeout();
    else if (eqls(str, S_CHANNEL)) return (String)getChannel();
    else if (eqls(str, S_DEAUTHREASON)) return (String)getDeauthReason();
    else if (eqls(str, S_PROBESPERSSID)) return (String)getProbesPerSSID();
    else if (eqls(str, S_MIN_DEAUTHS)) return (String)getMinDeauths();
    else if (eqls(str, S_DISPLAY_TIMEOUT)) return (String)getDisplayTimeout();

    // strings
    else if (eqls(str, S_SSID)) return getSSID();
    else if (eqls(str, S_LANG)) return getLang();
    else if (eqls(str, S_PASSWORD)) return getPassword();
    else if (eqls(str, S_MACAP)) return macToStr(getMacAP());
    else if (eqls(str, S_MACST)) return macToStr(getMacSt());
    else if (eqls(str, S_MAC)) return "AP: " + macToStr(getMacAP()) + ", Station: " + macToStr(getMacSt());
    else if (eqls(str, S_VERSION)) return getVersion();

    else {
        prnt(S_ERROR_NOT_FOUND);
        prntln(str);
    }

    return "";
}

// ===== GETTERS ===== //
String Settings::getVersion() {
    return DEAUTHER_VERSION;
}

uint16_t Settings::getDeauthsPerTarget() {
    return data.attack.deauths_per_target;
}

uint8_t Settings::getDeauthReason() {
    return data.attack.deauth_reason;
}

bool Settings::getBeaconChannel() {
    return data.attack.attack_all_ch;
}

bool Settings::getAutosave() {
    return data.autosave;
}

uint32_t Settings::getAutosaveTime() {
    return data.autosave_time;
}

bool Settings::getBeaconInterval() {
    return (int)data.attack.beacon_interval;
}

uint8_t Settings::getChannel() {
    return data.wifi.channel;
}

String Settings::getSSID() {
    return String(data.ap.ssid);
}

String Settings::getPassword() {
    return String(data.ap.password);
}

bool Settings::getCLI() {
    return data.cli.enabled;
}

bool Settings::getDisplayInterface() {
    return data.display.enabled;
}

bool Settings::getWebInterface() {
    return data.web.enabled;
}

uint16_t Settings::getChTime() {
    return data.sniffer.channel_time;
}

uint8_t* Settings::getMacSt() {
    return data.wifi.mac_st;
}

uint8_t* Settings::getMacAP() {
    return data.wifi.mac_ap;
}

bool Settings::getRandomTX() {
    return data.attack.random_tx;
}

uint32_t Settings::getAttackTimeout() {
    return data.attack.timeout;
}

bool Settings::getLedEnabled() {
    return data.led.enabled;
}

uint8_t Settings::getProbesPerSSID() {
    return data.attack.probe_frames_per_ssid;
}

bool Settings::getHidden() {
    return data.ap.hidden;
}

bool Settings::getCaptivePortal() {
    return data.web.captive_portal;
}

uint16_t Settings::getMinDeauths() {
    return data.sniffer.min_deauth_frames;
}

uint32_t Settings::getDisplayTimeout() {
    return data.display.timeout;
}

String Settings::getLang() {
    return data.web.lang;
}

bool Settings::getSerialEcho() {
    return data.cli.serial_echo;
}

bool Settings::getWebSpiffs() {
    return data.web.use_spiffs;
}

// ===== SETTERS ===== //

void Settings::setDeauthsPerTarget(uint8_t deauthsPerTarget) {
    data.attack.deauths_per_target = deauthsPerTarget;
    changed                        = true;
}

void Settings::setDeauthReason(uint8_t deauthReason) {
    data.attack.deauth_reason = deauthReason;
    changed                   = true;
}

void Settings::setBeaconChannel(bool beaconChannel) {
    data.attack.attack_all_ch = beaconChannel;
    changed                   = true;
}

void Settings::setAutosave(bool autosave) {
    data.autosave = autosave;
}

void Settings::setAutosaveTime(uint32_t autosaveTime) {
    data.autosave_time = autosaveTime;
}

void Settings::setBeaconInterval(bool beaconInterval) {
    data.attack.beacon_interval = (beacon_interval_t)(int)beaconInterval;
    changed                     = true;
}

void Settings::setChannel(uint8_t channel) {
    if ((channel >= 1) && (channel <= 14)) {
        data.wifi.channel = channel;
        changed           = true;

        setWifiChannel(channel);

        prnt(S_CHANNEL_CHANGE);
        prntln(channel);
    } else {
        prntln(S_CHANNEL_ERROR);
    }
}

void Settings::setSSID(String ssid) {
    if ((ssid.length() > 0) && (ssid.length() <= 32)) {
        ssid = fixUtf8(ssid);

        strncpy(data.ap.ssid, ssid.c_str(), 32);

        changed = true;
    } else {
        prntln(S_ERROR_SSID_LEN);
    }
}

void Settings::setPassword(String password) {
    if ((password.length() >= 8) && (password.length() <= 32)) {
        password = fixUtf8(password);

        strncpy(data.ap.password, password.c_str(), 64);

        changed = true;
    } else {
        prntln(S_ERROR_PASSWORD_LEN);
    }
}

void Settings::setCLI(bool cli) {
    data.cli.enabled = cli;
    changed          = true;
}

void Settings::setDisplayInterface(bool displayInterface) {
    data.display.enabled = displayInterface;
    changed              = true;
}

void Settings::setWebInterface(bool webInterface) {
    data.web.enabled = webInterface;
    changed          = true;
}

void Settings::setChTime(uint16_t chTime) {
    data.sniffer.channel_time = chTime;
    changed                   = true;
}

void Settings::setMacSt(String macStr) {
    uint8_t mac[6];

    if (eqls(macStr, S_RANDOM)) getRandomMac(mac);
    else strToMac(macStr, mac);

    setMacSt(mac);
}

bool Settings::setMacSt(uint8_t* macSt) {
    if (macSt[0] % 2 == 0) {
        memcpy(data.wifi.mac_st, macSt, 6);
        changed = true;
        return true;
    }
    return false;
}

void Settings::setMacAP(String macStr) {
    uint8_t mac[6];

    if (eqls(macStr, S_RANDOM)) getRandomMac(mac);
    else strToMac(macStr, mac);

    setMacAP(mac);
}

bool Settings::setMacAP(uint8_t* macAP) {
    if (macAP[0] % 2 == 0) {
        memcpy(data.wifi.mac_ap, macAP, 6);
        changed = true;
        return true;
    }
    return false;
}

void Settings::setRandomTX(bool randomTX) {
    data.attack.random_tx = randomTX;
    changed               = true;
}

void Settings::setAttackTimeout(uint32_t attackTimeout) {
    data.attack.timeout = attackTimeout;
    changed             = true;
}

void Settings::setLedEnabled(bool ledEnabled) {
    data.led.enabled = ledEnabled;
    changed          = true;
}

void Settings::setProbesPerSSID(uint8_t probesPerSSID) {
    if (probesPerSSID > 0) {
        data.attack.probe_frames_per_ssid = probesPerSSID;
        changed                           = true;
    }
}

void Settings::setHidden(bool hidden) {
    data.ap.hidden = hidden;
    changed        = true;
}

void Settings::setCaptivePortal(bool captivePortal) {
    data.web.captive_portal = captivePortal;
    changed                 = true;
}

void Settings::setMinDeauths(uint16_t minDeauths) {
    data.sniffer.min_deauth_frames = minDeauths;
    changed                        = true;
}

void Settings::setDisplayTimeout(uint32_t displayTimeout) {
    data.display.timeout = displayTimeout;
    changed              = true;
}

void Settings::setLang(String lang) {
    strncpy(data.web.lang, lang.c_str(), 2);
    changed = true;
}

void Settings::setSerialEcho(bool serialEcho) {
    data.cli.serial_echo = serialEcho;
    changed              = true;
}

void Settings::setWebSpiffs(bool webSpiffs) {
    data.web.use_spiffs = webSpiffs;
    changed             = true;
}