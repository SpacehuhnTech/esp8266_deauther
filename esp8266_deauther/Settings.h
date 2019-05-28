#ifndef Settings_h
#define Settings_h

// ====== Includes ====== //
// Libraries
#include <Arduino.h>  // Arduino String, Serial
#include <FS.h>       // SPIFFS

// Local files
#include "A_config.h" // Default Settings
#include "language.h" // prnt and prntln

// ====== Constants ====== //
#define SETTINGS_PATH "/settings.json"

// ====== External Functions ===== //
extern bool writeFile(String path, String& buf);
extern void getRandomMac(uint8_t* mac);
extern bool macValid(uint8_t* mac);

// ====== Strings ===== //
const char S_OK[] PROGMEM = "OK";
const char S_INVALID_HASH[] PROGMEM = "Invalid Hash - reseted to default";
const char S_SETTINGS_LOADED[] PROGMEM = "Loading settings...";
const char S_SETTINGS_RESETED[] PROGMEM = "Settings reseted";
const char S_SETTINGS_SAVED[] PROGMEM = "Settings saved in ";
const char S_ERROR_SAVING[] PROGMEM = "ERROR: saving ";
const char S_SETTINGS_HEADER[] PROGMEM = "[========== Settings ==========]";
const char S_CHANGED_SETTING[] PROGMEM = "Changed setting ";

// ===== JSON Strings ====== //
// General
const char S_JSON_TRUE[] PROGMEM = "true";
const char S_JSON_FALSE[] PROGMEM = "false";

// Version
const char S_JSON_VERSION[] PROGMEM = "version";

// Autosave
const char S_JSON_AUTOSAVE[] PROGMEM = "autosave";
const char S_JSON_AUTOSAVETIME[] PROGMEM = "autosavetime";

// Attack
const char S_JSON_BEACONCHANNEL[] PROGMEM = "beaconchannel";
const char S_JSON_RANDOMTX[] PROGMEM = "randomTX";
const char S_JSON_ATTACKTIMEOUT[] PROGMEM = "attacktimeout";
const char S_JSON_DEAUTHSPERTARGET[] PROGMEM = "deauthspertarget";
const char S_JSON_DEAUTHREASON[] PROGMEM = "deauthReason";
const char S_JSON_BEACONINTERVAL[] PROGMEM = "beaconInterval";
const char S_JSON_PROBESPERSSID[] PROGMEM = "probesPerSSID";

// WiFi
const char S_JSON_CHANNEL[] PROGMEM = "channel";
const char S_JSON_MACST[] PROGMEM = "macSt";
const char S_JSON_MACAP[] PROGMEM = "macAP";

// Sniffer
const char S_JSON_CHTIME[] PROGMEM = "chtime";
const char S_JSON_MIN_DEAUTHS[] PROGMEM = "minDeauths";

// AP
const char S_JSON_SSID[] PROGMEM = "ssid";
const char S_JSON_PASSWORD[] PROGMEM = "password";
const char S_JSON_HIDDEN[] PROGMEM = "hidden";
const char S_JSON_IP[] PROGMEM = "ip";

// Web
const char S_JSON_WEBINTERFACE[] PROGMEM = "webinterface";
const char S_JSON_CAPTIVEPORTAL[] PROGMEM = "captivePortal";
const char S_JSON_WEB_SPIFFS[] PROGMEM = "webSpiffs";
const char S_JSON_LANG[] PROGMEM = "lang";

// CLI
const char S_JSON_SERIALINTERFACE[] PROGMEM = "serial";
const char S_JSON_SERIAL_ECHO[] PROGMEM = "serialEcho";

// LED
const char S_JSON_LEDENABLED[] PROGMEM = "led";

// Display
const char S_JSON_DISPLAYINTERFACE[] PROGMEM = "display";
const char S_JSON_DISPLAY_TIMEOUT[] PROGMEM = "displayTimeout";

// ===== VERSION ===== //
typedef struct version_t {
    uint8_t major    = DEAUTHER_VERSION_MAJOR;
    uint8_t minor    = DEAUTHER_VERSION_MINOR;
    uint8_t revision = DEAUTHER_VERSION_REVISION;
} version_t;

// ===== AUTOSAVE ===== //
typedef struct autosave_settings_t {
    bool     enabled = AUTOSAVE_ENABLED;
    uint32_t time    = AUTOSAVE_TIME;
} autosave_t;

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
    autosave_settings_t     autosave;
    attack_settings_t       attack;
    wifi_settings_t         wifi;
    sniffer_settings_t      sniffer;
    access_point_settings_t ap;
    web_settings_t          web;
    cli_settings_t          cli;
    led_settings_t          led;
    display_settings_t      display;
} settings_t;

// ===== CHECK SUM / HASH ====== //
typedef struct settings_hash_t {
    uint8_t hash[20];
} settings_hash_t;

// ===== SETTINGS ===== //
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

        const settings_t& getAllSettings();
        const version_t & getVersion();
        const autosave_settings_t& getAutosaveSettings();
        const attack_settings_t  & getAttackSettings();
        const wifi_settings_t    & getWifiSettings();
        const sniffer_settings_t & getSnifferSettings();
        const access_point_settings_t& getAccessPointSettings();
        const web_settings_t& getWebSettings();
        const cli_settings_t& getCLISettings();
        const led_settings_t& getLEDSettings();
        const display_settings_t& getDisplaySettings();

        void setAllSettings(settings_t& settings);
        void setAutosaveSettings(const autosave_settings_t& autosave);
        void setAttackSettings(const attack_settings_t& attack);
        void setWifiSettings(const wifi_settings_t& wifi);
        void setSnifferSettings(const sniffer_settings_t& sniffer);
        void setAccessPointSettings(const access_point_settings_t& ap);
        void setWebSettings(const web_settings_t& web);
        void setCLISettings(const cli_settings_t& cli);
        void setLEDSettings(const led_settings_t& led);
        void setDisplaySettings(const display_settings_t& display);
};

#endif // ifndef Settings_h