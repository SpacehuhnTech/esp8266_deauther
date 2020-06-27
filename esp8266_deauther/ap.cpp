/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "ap.h"
#include "debug.h"
#include "strh.h"
#include "scan.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>

extern "C" {
#include "user_interface.h"
}

typedef struct ap_settings_t {
    bool enabled;
    bool paused;
    char ssid[33];
    char pswd[65];
    uint8_t ch;
    bool hidden;
    uint8_t bssid[6];
} ap_settings_t;

namespace ap {
    // ========== PRIVATE ========= //
    ESP8266WebServer server(80);
    DNSServer dnsServer;
    IPAddress apIP(192, 168, 4, 1);
    IPAddress netMsk(255, 255, 255, 0);
    ap_settings_t ap_settings;

    void print_client_list() {
        unsigned char number_client;
        struct station_info* stat_info;
        
        struct ip_addr* IPaddress;
        IPAddress address;
        
        number_client= wifi_softap_get_station_num();
        stat_info = wifi_softap_get_station_info();

        debuglnF("[ ========= Clients ========= ]");

        if(number_client == 0) {
            debuglnF("No clients connected");
            return;
        }
        debuglnF("ID IP-Address      MAC-Address");
        debuglnF("====================================");
        
        int i { 0 };

        while (stat_info) {
            IPaddress = &stat_info->ip;
            address = IPaddress->addr;
            
            debug(strh::right(2, String(i)));
            debug(' ');
            debug(strh::left(15, address.toString()));
            debug(' ');
            debug(strh::left(17, strh::mac(stat_info->bssid)));
            debugln();

            stat_info = STAILQ_NEXT(stat_info, next);
            ++i;
        }

        debuglnF("====================================");
        debugln();
    }

    void handle_404() {
        server.send(200, "text/plain", "Good morning, friend!");
        
        debugF("> Client requests ");
        debugln(server.uri());
        debugln();

        print_client_list();
    }

    // ========== PUBLIC ========= //
    void start(String& ssid, String& pswd, bool hidden, uint8_t ch, uint8_t* bssid) {
        if (ssid.length() == 0) {
            debugln("ERROR: SSID empty");
            return;
        }

        if (ssid.length() > 32) {
            debugln("WARNING: SSID longer than 32 characters");
            ssid = ssid.substring(0, 32);
        }

        if (pswd.length() > 0 && pswd.length() < 8) {
            debugln("WARNING: Password must have at least 8 characters");
            pswd = String();
        }

        if(ch < 1 || ch > 14) {
            debugln("WARNING: Channel must be between 1-14");
            ch = 1;
        }

        scan::stopST();

        ap_settings.enabled = true;
        ap_settings.paused = false;
        strncpy(ap_settings.ssid, ssid.c_str(), 32);
        strncpy(ap_settings.pswd, pswd.c_str(), 64);
        ap_settings.hidden = hidden;
        ap_settings.ch = ch;
        memcpy(ap_settings.bssid, bssid, 6);

        wifi_set_macaddr(SOFTAP_IF, ap_settings.bssid);
        WiFi.softAPConfig(apIP, apIP, netMsk);
        WiFi.softAP(ap_settings.ssid, ap_settings.pswd, ap_settings.ch, ap_settings.hidden);

        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(53, "*", apIP);

        MDNS.begin("deauth.me");

        server.onNotFound(handle_404);

        server.begin();

        debuglnF("[ ========= Access Point ========= ]");

        debugF("SSID:      ");
        debugln(ap_settings.ssid);

        debugF("Password:  ");
        debugln(ap_settings.pswd);

        debugF("Mode:      ");
        debugln(ap_settings.pswd == '\0' ? "WPA2" : "Open");

        debugF("Hidden:    ");
        debugln(strh::boolean(ap_settings.hidden));

        debugF("Channel:   ");
        debugln(ap_settings.ch);

        debugF("BSSID:     ");
        debugln(strh::mac(ap_settings.bssid));

        debugln();

        debuglnF("Type 'stop ap' to stop the access point");
    }

    void stop() {
        if (ap_settings.enabled || ap_settings.paused) {
            WiFi.persistent(false);
            WiFi.disconnect(true);
            wifi_set_opmode(STATION_MODE);

            ap_settings.enabled = false;
            ap_settings.paused = false;
            
            debuglnF("> Stopped access point");
            debugln();
        }
    }
    
    void pause() {
        if (ap_settings.enabled && !ap_settings.paused) {
            stop();
            ap_settings.paused = true;
        }
    }

    void resume() {
        if (!ap_settings.enabled && ap_settings.paused) {
            WiFi.softAP(ap_settings.ssid, ap_settings.pswd, ap_settings.ch, ap_settings.hidden);

            ap_settings.enabled = true;
            ap_settings.paused = false;
            
            debuglnF("> Resumed access point");
            debugln();
        }
    }

    bool paused() {
        return ap_settings.paused;
    }

    void update() {
        if(ap_settings.enabled) {
            server.handleClient();
            MDNS.update();
            dnsServer.processNextRequest();
        }
    }
}