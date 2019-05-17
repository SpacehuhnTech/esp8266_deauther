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
    str += String(value ? S_JSON_TRUE : S_JSON_FALSE);
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

    for (int i = 0; i<len; i++) {
        if (i > 0) str += ':';
        if (byteArr[i] < 0x10) str += '0';
        str += String(byteArr[i], HEX);
    }

    str += ',';
}

void jsonDec(String& str, const char* name, uint8_t* byteArr, int len) {
    str += '"';
    str += String(name);
    str += '"';
    str += ':';

    for (int i = 0; i<len; i++) {
        if (i > 0) str += '.';
        str += String(byteArr[i]);
    }

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
    settingsJson.replace("\"", "");
    settingsJson.replace("{", "");
    settingsJson.replace("}", "");
    settingsJson.replace(",", "\r\n");

    prntln(S_SETTINGS_HEADER);
    prntln(settingsJson);
}

/*
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
 */
// ===== GETTERS ===== //

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

/*
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
    return data.autosave.enabled;
   }

   uint32_t Settings::getAutosaveTime() {
    return data.autosave.time;
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
 */
// ===== SETTERS ===== //

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

/*
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
    data.autosave.enabled = autosave;
   }

   void Settings::setAutosaveTime(uint32_t autosaveTime) {
    data.autosave.time = autosaveTime;
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
   }*/