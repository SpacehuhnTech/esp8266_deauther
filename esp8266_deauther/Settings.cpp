#include "Settings.h"

Settings::Settings() {
    macSt = (uint8_t*)malloc(6);
    macAP = (uint8_t*)malloc(6);
}

void Settings::load() {
    DynamicJsonBuffer jsonBuffer(4000);

    // check & read file
    String json = getJsonStr();

    checkFile(FILE_PATH, json);
    JsonObject& data = parseJSONFile(FILE_PATH, jsonBuffer);

    // VERSION
    version = data.get<String>("version");

    // AP
    if (data.containsKey(keyword(S_SSID))) setSSID(data.get<String>(keyword(S_SSID)));
    if (data.containsKey(keyword(S_PASSWORD))) setPassword(data.get<String>(keyword(S_PASSWORD)));
    if (data.containsKey(keyword(S_CHANNEL))) setChannel(data.get<uint8_t>(keyword(S_CHANNEL)));
    if (data.containsKey(keyword(S_HIDDEN))) setHidden(data.get<bool>(keyword(S_HIDDEN)));
    if (data.containsKey(keyword(S_CAPTIVEPORTAL))) setCaptivePortal(data.get<bool>(keyword(S_CAPTIVEPORTAL)));

    // GENERAL
    if (data.containsKey(keyword(S_LANG))) setLang(data.get<String>(keyword(S_LANG)));
    if (data.containsKey(keyword(S_DISPLAYINTERFACE))) setDisplayInterface(data.get<bool>(keyword(S_DISPLAYINTERFACE)));
    if (data.containsKey(keyword(S_DISPLAY_TIMEOUT))) setDisplayTimeout(data.get<uint32_t>(keyword(S_DISPLAY_TIMEOUT)));
    if (data.containsKey(keyword(S_SERIALINTERFACE))) setCLI(data.get<bool>(keyword(S_SERIALINTERFACE)));
    if (data.containsKey(keyword(S_SERIAL_ECHO))) setSerialEcho(data.get<bool>(keyword(S_SERIAL_ECHO)));
    if (data.containsKey(keyword(S_WEBINTERFACE))) setWebInterface(data.get<bool>(keyword(S_WEBINTERFACE)));
    if (data.containsKey(keyword(S_WEB_SPIFFS))) setWebSpiffs(data.get<bool>(keyword(S_WEB_SPIFFS)));
    if (data.containsKey(keyword(S_LEDENABLED))) setLedEnabled(data.get<bool>(keyword(S_LEDENABLED)));
    if (data.containsKey(keyword(S_MACAP))) setMacAP(data.get<String>(keyword(S_MACAP)));
    if (data.containsKey(keyword(S_MACST))) setMacSt(data.get<String>(keyword(S_MACST)));

    // SCAN
    if (data.containsKey(keyword(S_CHTIME))) setChTime(data.get<uint16_t>(keyword(S_CHTIME)));
    if (data.containsKey(keyword(S_MIN_DEAUTHS))) setMinDeauths(data.get<uint16_t>(keyword(S_MIN_DEAUTHS)));

    // ATTACK
    if (data.containsKey(keyword(S_ATTACKTIMEOUT))) setAttackTimeout(data.get<uint32_t>(keyword(S_ATTACKTIMEOUT)));
    if (data.containsKey(keyword(S_FORCEPACKETS))) setForcePackets(data.get<uint8_t>(keyword(S_FORCEPACKETS)));
    if (data.containsKey(keyword(S_DEAUTHSPERTARGET))) setDeauthsPerTarget(data.get<uint16_t>(keyword(
                                                                                                  S_DEAUTHSPERTARGET)));

    if (data.containsKey(keyword(S_DEAUTHREASON))) setDeauthReason(data.get<uint8_t>(keyword(S_DEAUTHREASON)));
    if (data.containsKey(keyword(S_BEACONCHANNEL))) setBeaconChannel(data.get<bool>(keyword(S_BEACONCHANNEL)));
    if (data.containsKey(keyword(S_BEACONINTERVAL))) setBeaconInterval(data.get<bool>(keyword(S_BEACONINTERVAL)));
    if (data.containsKey(keyword(S_RANDOMTX))) setRandomTX(data.get<bool>(keyword(S_RANDOMTX)));
    if (data.containsKey(keyword(S_PROBESPERSSID))) setProbesPerSSID(data.get<uint8_t>(keyword(S_PROBESPERSSID)));

    if (version != VERSION) {
        // reset();
        copyWebFiles(true);
        version = VERSION;
        changed = true;
    }

    prnt(S_SETTINGS_LOADED);
    prntln(FILE_PATH);

    // check and fix mac
    if (!macValid(macSt)) getRandomMac(macSt);

    if (!macValid(macAP)) getRandomMac(macAP);

    save(true); // force saving
}

void Settings::load(String filepath) {
    String tmp = FILE_PATH;

    FILE_PATH = filepath;
    load();
    FILE_PATH = tmp;
}

void Settings::reset() {
    // VERSION
    version = VERSION;

    // AP
    setSSID("pwned");
    setPassword("deauther");
    setChannel(9);
    setHidden(false);
    setCaptivePortal(true);

    // GENERAL
    setLang("en");
    setAutosave(true);
    setAutosaveTime(10000);
    setDisplayInterface(USE_DISPLAY);
    setDisplayTimeout(600);
    setCLI(true);
    setSerialEcho(true);
    setWebInterface(true);
    setWebSpiffs(false);
    setLedEnabled(true);
    wifi_get_macaddr(STATION_IF, macSt);
    wifi_get_macaddr(SOFTAP_IF, macAP);

    // SCAN
    setChTime(384);
    setMinDeauths(3);

    // ATTACK
    setAttackTimeout(600);
    setForcePackets(3);
    setDeauthsPerTarget(20);
    setDeauthReason(1);
    setBeaconChannel(false);
    setBeaconInterval(false);
    setRandomTX(false);
    setProbesPerSSID(1);

    prntln(S_SETTINGS_RESETED);
}

String Settings::getJsonStr() {
    DynamicJsonBuffer jsonBuffer(4000);
    JsonObject& data = jsonBuffer.createObject();

    // Version
    data.set("version", VERSION);

    // AP
    data.set(keyword(S_SSID), ssid);
    data.set(keyword(S_PASSWORD), password);
    data.set(keyword(S_CHANNEL), channel);
    data.set(keyword(S_HIDDEN), hidden);
    data.set(keyword(S_CAPTIVEPORTAL), captivePortal);

    // GENERAL
    data.set(keyword(S_LANG), lang);
    data.set(keyword(S_AUTOSAVE), autosave);
    data.set(keyword(S_AUTOSAVETIME), autosaveTime);
    data.set(keyword(S_DISPLAYINTERFACE), displayInterface);
    data.set(keyword(S_DISPLAY_TIMEOUT), displayTimeout);
    data.set(keyword(S_SERIALINTERFACE), cli);
    data.set(keyword(S_SERIAL_ECHO), serialEcho);
    data.set(keyword(S_WEBINTERFACE), webInterface);
    data.set(keyword(S_WEB_SPIFFS), webSpiffs);
    data.set(keyword(S_LEDENABLED), ledEnabled);
    data.set(keyword(S_MACAP), macToStr(getMacAP()));
    data.set(keyword(S_MACST), macToStr(getMacSt()));

    // SCAN
    data.set(keyword(S_CHTIME), chTime);
    data.set(keyword(S_MIN_DEAUTHS), minDeauths);

    // ATTACK
    data.set(keyword(S_ATTACKTIMEOUT), attackTimeout);
    data.set(keyword(S_FORCEPACKETS), forcePackets);
    data.set(keyword(S_DEAUTHSPERTARGET), deauthsPerTarget);
    data.set(keyword(S_DEAUTHREASON), deauthReason);
    data.set(keyword(S_BEACONCHANNEL), beaconChannel);
    data.set(keyword(S_BEACONINTERVAL), beaconInterval);
    data.set(keyword(S_RANDOMTX), randomTX);
    data.set(keyword(S_PROBESPERSSID), probesPerSSID);

    String buf;
    data.printTo(buf);

    return buf;
}

void Settings::save(bool force) {
    if (force || changed) {
        String buf = getJsonStr();

        if (writeFile(FILE_PATH, buf)) {
            prnt(S_SETTINGS_SAVED);
            prntln(FILE_PATH);
            changed = false;
        } else {
            prnt(F("ERROR: saving "));
            prntln(FILE_PATH);
        }
    }
}

void Settings::save(bool force, String filepath) {
    String tmp = FILE_PATH;

    FILE_PATH = filepath;
    save(force);
    FILE_PATH = tmp;
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
    else if (eqls(str, S_FORCEPACKETS)) setForcePackets(value.toInt());
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
    else if (eqls(str, S_BEACONCHANNEL)) return b2s(beaconChannel);
    else if (eqls(str, S_AUTOSAVE)) return b2s(autosave);
    else if (eqls(str, S_BEACONINTERVAL)) return b2s(beaconInterval);
    else if (eqls(str, S_SERIALINTERFACE)) return b2s(cli);
    else if (eqls(str, S_DISPLAYINTERFACE)) return b2s(displayInterface);
    else if (eqls(str, S_WEBINTERFACE)) return b2s(webInterface);
    else if (eqls(str, S_RANDOMTX)) return b2s(randomTX);
    else if (eqls(str, S_LEDENABLED)) return b2s(ledEnabled);
    else if (eqls(str, S_HIDDEN)) return b2s(hidden);
    else if (eqls(str, S_CAPTIVEPORTAL)) return b2s(captivePortal);
    else if (eqls(str, S_SERIAL_ECHO)) return b2s(serialEcho);
    else if (eqls(str, S_WEB_SPIFFS)) return b2s(webSpiffs);

    // integer
    else if (eqls(str, S_FORCEPACKETS)) return (String)forcePackets;
    else if (eqls(str, S_AUTOSAVETIME)) return (String)autosaveTime;
    else if (eqls(str, S_DEAUTHSPERTARGET)) return (String)deauthsPerTarget;
    else if (eqls(str, S_CHTIME)) return (String)chTime;
    else if (eqls(str, S_ATTACKTIMEOUT)) return (String)attackTimeout;
    else if (eqls(str, S_CHANNEL)) return (String)channel;
    else if (eqls(str, S_DEAUTHREASON)) return (String)deauthReason;
    else if (eqls(str, S_PROBESPERSSID)) return (String)probesPerSSID;
    else if (eqls(str, S_MIN_DEAUTHS)) return (String)minDeauths;
    else if (eqls(str, S_DISPLAY_TIMEOUT)) return (String)displayTimeout;

    // strings
    else if (eqls(str, S_SSID)) return ssid;
    else if (eqls(str, S_LANG)) return lang;
    else if (eqls(str, S_PASSWORD)) return password;
    else if (eqls(str, S_MACAP)) return macToStr(getMacAP());
    else if (eqls(str, S_MACST)) return macToStr(getMacSt());
    else if (eqls(str, S_MAC)) return "AP: " + macToStr(macAP) + ", Station: " + macToStr(macSt);
    else if (eqls(str, S_VERSION)) return version;

    else {
        prnt(S_ERROR_NOT_FOUND);
        prntln(str);
    }

    return "";
}

// ===== GETTERS ===== //
String Settings::getVersion() {
    return version;
}

uint16_t Settings::getDeauthsPerTarget() {
    return deauthsPerTarget;
}

uint8_t Settings::getDeauthReason() {
    return deauthReason;
}

bool Settings::getBeaconChannel() {
    return beaconChannel;
}

uint8_t Settings::getForcePackets() {
    return forcePackets;
}

bool Settings::getAutosave() {
    return autosave;
}

uint32_t Settings::getAutosaveTime() {
    return autosaveTime;
}

bool Settings::getBeaconInterval() {
    return beaconInterval;
}

uint8_t Settings::getChannel() {
    return channel;
}

String Settings::getSSID() {
    return ssid;
}

String Settings::getPassword() {
    return password;
}

bool Settings::getCLI() {
    return cli;
}

bool Settings::getDisplayInterface() {
    return displayInterface;
}

bool Settings::getWebInterface() {
    return webInterface;
}

uint16_t Settings::getChTime() {
    return chTime;
}

uint8_t* Settings::getMacSt() {
    return macSt;
}

uint8_t* Settings::getMacAP() {
    return macAP;
}

bool Settings::getRandomTX() {
    return randomTX;
}

uint32_t Settings::getAttackTimeout() {
    return attackTimeout;
}

bool Settings::getLedEnabled() {
    return ledEnabled;
}

uint8_t Settings::getProbesPerSSID() {
    return probesPerSSID;
}

bool Settings::getHidden() {
    return hidden;
}

bool Settings::getCaptivePortal() {
    return captivePortal;
}

uint16_t Settings::getMinDeauths() {
    return minDeauths;
}

uint32_t Settings::getDisplayTimeout() {
    return displayTimeout;
}

String Settings::getLang() {
    return lang;
}

bool Settings::getSerialEcho() {
    return serialEcho;
}

bool Settings::getWebSpiffs() {
    return webSpiffs;
}

// ===== SETTERS ===== //

void Settings::setDeauthsPerTarget(uint16_t deauthsPerTarget) {
    Settings::deauthsPerTarget = deauthsPerTarget;

    changed = true;
}

void Settings::setDeauthReason(uint8_t deauthReason) {
    Settings::deauthReason = deauthReason;

    changed = true;
}

void Settings::setBeaconChannel(bool beaconChannel) {
    Settings::beaconChannel = beaconChannel;

    changed = true;
}

void Settings::setForcePackets(uint8_t forcePackets) {
    if (forcePackets > 0) {
        Settings::forcePackets = forcePackets;
        changed                = true;
    }
}

void Settings::setAutosave(bool autosave) {
    Settings::autosave = autosave;

    changed = true;
}

void Settings::setAutosaveTime(uint32_t autosaveTime) {
    Settings::autosaveTime = autosaveTime;

    changed = true;
}

void Settings::setBeaconInterval(bool beaconInterval) {
    Settings::beaconInterval = beaconInterval;

    changed = true;
}

void Settings::setChannel(uint8_t channel) {
    if ((channel >= 1) && (channel <= 14)) {
        Settings::channel = channel;
        setWifiChannel(channel);
        changed = true;
        prnt(S_CHANNEL_CHANGE);
        prntln(channel);
    } else {
        prntln(S_CHANNEL_ERROR);
    }
}

void Settings::setSSID(String ssid) {
    if ((ssid.length() > 0) && (ssid.length() <= 32)) {
        ssid           = fixUtf8(ssid);
        Settings::ssid = ssid;
        changed        = true;
    } else {
        prntln(S_ERROR_SSID_LEN);
    }
}

void Settings::setPassword(String password) {
    if ((password.length() >= 8) && (password.length() <= 32)) {
        password           = fixUtf8(password);
        Settings::password = password;
        changed            = true;
    } else {
        prntln(S_ERROR_PASSWORD_LEN);
    }
}

void Settings::setCLI(bool cli) {
    Settings::cli = cli;

    changed = true;
}

void Settings::setDisplayInterface(bool displayInterface) {
    Settings::displayInterface = displayInterface;

    changed = true;
}

void Settings::setWebInterface(bool webInterface) {
    Settings::webInterface = webInterface;

    changed = true;
}

void Settings::setChTime(uint16_t chTime) {
    Settings::chTime = chTime;

    changed = true;
}

void Settings::setMacSt(String macStr) {
    uint8_t mac[6];

    if (eqls(macStr, S_RANDOM)) getRandomMac(mac);
    else strToMac(macStr, mac);

    setMacSt(mac);
}

bool Settings::setMacSt(uint8_t* macSt) {
    if (macSt[0] % 2 == 0) {
        memcpy(Settings::macSt, macSt, 6);
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
        memcpy(Settings::macAP, macAP, 6);
        changed = true;
        return true;
    }
    return false;
}

void Settings::setRandomTX(bool randomTX) {
    Settings::randomTX = randomTX;

    changed = true;
}

void Settings::setAttackTimeout(uint32_t attackTimeout) {
    Settings::attackTimeout = attackTimeout;

    changed = true;
}

void Settings::setLedEnabled(bool ledEnabled) {
    Settings::ledEnabled = ledEnabled;

    changed = true;
}

void Settings::setProbesPerSSID(uint8_t probesPerSSID) {
    if (probesPerSSID > 0) {
        Settings::probesPerSSID = probesPerSSID;
        changed                 = true;
    }
}

void Settings::setHidden(bool hidden) {
    Settings::hidden = hidden;

    changed = true;
}

void Settings::setCaptivePortal(bool captivePortal) {
    Settings::captivePortal = captivePortal;

    changed = true;
}

void Settings::setMinDeauths(uint16_t minDeauths) {
    Settings::minDeauths = minDeauths;

    changed = true;
}

void Settings::setDisplayTimeout(uint32_t displayTimeout) {
    Settings::displayTimeout = displayTimeout;

    changed = true;
}

void Settings::setLang(String lang) {
    Settings::lang = lang;

    changed = true;
}

void Settings::setSerialEcho(bool serialEcho) {
    Settings::serialEcho = serialEcho;

    changed = true;
}

void Settings::setWebSpiffs(bool webSpiffs) {
    Settings::webSpiffs = webSpiffs;

    changed = true;
}