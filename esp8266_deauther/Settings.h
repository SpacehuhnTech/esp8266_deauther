#ifndef Settings_h
#define Settings_h

#include <Arduino.h> // String

// Load and save JSON files
#include <ArduinoJSON.h>
#include <FS.h>

#include "A_config.h" // Default Settings
#include "language.h" // Debug strings

#define SETTINGS_PATH "/settings.json"

extern void checkFile(String path, String data);
extern JsonVariant parseJSONFile(String path, DynamicJsonBuffer& jsonBuffer);
extern bool writeFile(String path, String& buf);
extern void saveJSONFile(String path, JsonObject& root);
extern String macToStr(uint8_t* mac);
extern void getRandomMac(uint8_t* mac);
extern bool strToMac(String macStr, uint8_t* mac);
extern void setWifiChannel(uint8_t ch);
extern String fixUtf8(String str);
extern void copyWebFiles(bool force);
extern bool macValid(uint8_t* mac);
extern String bytesToStr(uint8_t* b, uint32_t size);

// ===== VERSION ===== //
typedef struct version_t {
    uint8_t major    = DEAUTHER_VERSION_MAJOR;
    uint8_t minor    = DEAUTHER_VERSION_MINOR;
    uint8_t revision = DEAUTHER_VERSION_REVISION;
} version_t;

// ===== ATTACK ===== //
typedef enum beacon_interval_t {
    INTERVAL_1S    = 0,
    INTERVAL_100MS = 1
} beacon_interval_t;

typedef struct attack_settings_t {
    // General
    bool     attack_all_ch = ATTACK_ALL_CH;
    bool     random_tx     = RANDOM_TX;
    uint32_t timeout       = ATTACK_TIMEOUT;

    // Deauth
    uint8_t deauths_per_target = DEAUTHS_PER_TARGET;
    uint8_t deauth_reason      = DEAUTH_REASON;

    // Beacon
    beacon_interval_t beacon_interval = (beacon_interval_t)(int)BEACON_INTERVAL_100MS;

    // Probe
    uint8_t probe_frames_per_ssid = PROBE_FRAMES_PER_SSID;
} attack_settings_t;

// ====== WIFI ====== //
typedef struct wifi_settings_t {
    uint8_t channel = 1;
    uint8_t mac_st[6];
    uint8_t mac_ap[6];
} wifi_settings_t;

// ===== SNIFFER ===== //
typedef struct sniffer_settings_t {
    uint16_t channel_time      = CH_TIME;
    uint16_t min_deauth_frames = MIN_DEAUTH_FRAMES;
} sniffer_settings_t;

// ===== ACCESS POINT ===== //
typedef struct access_point_settings_t {
    char    ssid[33]     = AP_SSID;
    char    password[65] = AP_PASSWD;
    bool    hidden       = AP_HIDDEN;
    uint8_t ip[4]        = AP_IP_ADDR;
} access_point_settings_t;

// ===== WEB INTERFACE ===== //
typedef struct web_settings_t {
    bool enabled        = WEB_ENABLED;
    bool captive_portal = WEB_CAPTIVE_PORTAL;
    bool use_spiffs     = WEB_USE_SPIFFS;
    char lang[3]        = DEFAULT_LANG;
} web_settings_t;

// ===== CLI ===== //
typedef struct cli_settings_t {
    bool enabled     = CLI_ENABLED;
    bool serial_echo = CLI_ECHO;
} cli_settings_t;

// ===== LED ===== //
typedef struct led_settings_t {
    bool enabled = USE_LED;
} led_settings_t;

// ===== DISPLAY ===== //
typedef struct display_settings_t {
    bool     enabled = USE_DISPLAY;
    uint32_t timeout = DISPLAY_TIMEOUT;
} display_settings_t;

// ===== SETTINGS ===== //
typedef struct settings_t {
    version_t               version;
    attack_settings_t       attack;
    wifi_settings_t         wifi;
    sniffer_settings_t      sniffer;
    access_point_settings_t ap;
    web_settings_t          web;
    cli_settings_t          cli;
    led_settings_t          led;
    display_settings_t      display;

    bool     autosave;
    uint32_t autosave_time;
} settings_t;

// ===== CHECK SUM / HASH ====== //
typedef struct settings_hash_t {
    uint8_t hash[20];
} settings_hash_t;

class Settings {
    private:
        settings_t data;

        bool changed = false;

        settings_hash_t calcHash(settings_t data);

        String getJsonStr();

    public:
        void load();
        void save(bool force = false);

        void reset();
        void print();

        void set(const char* str, String value);
        String get(const char* str);

        String getVersion();
        uint16_t getDeauthsPerTarget();
        uint8_t getDeauthReason();
        bool getBeaconChannel();
        bool getAutosave();
        uint32_t getAutosaveTime();
        bool getBeaconInterval();
        uint8_t getChannel();
        String getSSID();
        String getPassword();
        bool getCLI();
        bool getDisplayInterface();
        bool getWebInterface();
        uint16_t getChTime();
        uint8_t* getMacSt();
        uint8_t* getMacAP();
        bool getRandomTX();
        uint32_t getAttackTimeout();
        bool getLedEnabled();
        uint8_t getProbesPerSSID();
        bool getHidden();
        bool getCaptivePortal();
        uint16_t getMinDeauths();
        uint32_t getDisplayTimeout();
        String getLang();
        bool getSerialEcho();
        bool getWebSpiffs();

        void setDeauthsPerTarget(uint8_t deauthsPerTarget);
        void setDeauthReason(uint8_t deauthReason);
        void setBeaconChannel(bool beaconChannel);
        void setAutosave(bool autosave);
        void setAutosaveTime(uint32_t autosaveTime);
        void setBeaconInterval(bool beaconInterval);
        void setChannel(uint8_t channel);
        void setSSID(String ssid);
        void setPassword(String password);
        void setCLI(bool cli);
        void setDisplayInterface(bool displayInterface);
        void setWebInterface(bool webInterface);
        void setChTime(uint16_t chTime);
        void setMacSt(String macStr);
        bool setMacSt(uint8_t* macSt);
        void setMacAP(String macStr);
        bool setMacAP(uint8_t* macAP);
        void setRandomTX(bool randomTX);
        void setAttackTimeout(uint32_t attackTimeout);
        void setLedEnabled(bool ledEnabled);
        void setProbesPerSSID(uint8_t probesPerSSID);
        void setHidden(bool hidden);
        void setCaptivePortal(bool captivePortal);
        void setMinDeauths(uint16_t minDeauths);
        void setDisplayTimeout(uint32_t displayTimeout);
        void setLang(String lang);
        void setSerialEcho(bool serialEcho);
        void setWebSpiffs(bool webSpiffs);
};

#endif // ifndef Settings_h