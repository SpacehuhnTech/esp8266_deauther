/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "scan.h"
#include "debug.h"
#include "strh.h"
#include "vendor.h"
#include "cli.h"
#include "mac.h"
#include "attack.h"

#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}
typedef struct scan_data_t {
    bool          ap;
    bool          st;
    bool          auth;
    uint16_t      channels;
    unsigned long ch_time;
    unsigned long timeout;
    uint8_t       bssid[5];
    bool          silent;

    uint8_t       num_of_channels;
    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkt_time;
    unsigned long ch_update_time;
    unsigned long pkts_received;
    unsigned long pkts_per_second;

    AccessPointList ap_list;
    StationList     st_list;
} scan_data_t;

namespace scan {
    // ===== PRIVATE ===== //
    scan_data_t data;

    void printChannel(uint8_t ch) {
        if (!data.silent) {
            debug("Sniff channel ");
            debug(ch);

            if (data.ch_time > 0) {
                debug(" (");
                if (data.ch_time < 1000) {
                    debug(data.ch_time);
                    debug(" ms)");
                } else {
                    debug(data.ch_time/1000);
                    debug(" s)");
                }
            }

            debugln();
        }
    }

    void setNextChannel() {
        if (!data.num_of_channels > 0) return;

        uint8_t ch = wifi_get_channel();

        do {
            if (++ch > 14) ch = 1;
            if ((data.channels >> (ch-1)) & 0x01) {
                printChannel(ch);
                wifi_set_channel(ch);
                break;
            }
        } while (true);
    }

    Station* register_station(uint8_t* mac, AccessPoint* ap) {
        Station* tmp = data.st_list.search(mac);

        if (!tmp && data.st_list.push(mac)) {
            tmp = data.st_list.search(mac);

            if (!data.silent && ap) {
                // debug(strh::mac(mac));
                // debugln(" new station");
                // if (ap) {
                debug(strh::mac(mac));
                debug(" connected to \"");
                debug(ap->getSSID());
                debugln('"');
                // }
            }
        }

        if (!tmp) {
            // Push error
            return NULL;
        }

        if (ap) tmp->setAccessPoint(ap);
        tmp->newPkt();

        return tmp;
    }

    void station_sniffer(uint8_t* buf, uint16_t len) {
        // drop frames that are too short to have a valid MAC header
        if (len < 28) return;

        uint8_t type = buf[12];

        // drop ... frames
        if (
            (type == 0xc0) || // deauthentication
            (type == 0xa0) || // disassociation
            (type == 0x80) || // beacon frames
            (type == 0x50)    // probe response
            ) return;

        // only allow data frames
        // if(buf[12] != 0x08 && buf[12] != 0x88) return;

        uint8_t* mac_to   = &buf[16]; // To (Receiver)
        uint8_t* mac_from = &buf[22]; // From (Transmitter)

        // broadcast probe request
        if ((type == 0x40) && (buf[12+25] > 0)) {
            const char* ssid = (const char*)&buf[12+26];
            uint8_t     len  = buf[12+25];

            Station* st = register_station(mac_from, NULL);

            if (st && st->addProbe(ssid, len)) {
                if (!data.silent) {
                    debug(strh::mac(mac_from));
                    debug(" probe \"");

                    for (uint8_t i = 0; i<len; ++i) debug(char(ssid[i]));
                    debugln("\"");
                }
            }
        }
        // authentication
        else if (data.auth && (type == 0xb0) && (memcmp(mac_to, data.bssid, 5) == 0)) {
            Station* st = register_station(mac_from, NULL);

            if (st && st->addAuth(mac_to[5])) {
                if (!data.silent) {
                    debug(strh::mac(mac_from));
                    debug(" auth \"");
                    // debugln(strh::mac(mac_to));
                    debug(attack::getBeacon(mac_to[5]));
                    debugln("\"");
                }
            }
        }
        // anything else that isn't a broadcast frame
        else if (!mac::multicast(mac_to)) {
            AccessPoint* ap = data.ap_list.search(mac_to);

            if (ap) {
                // From station to access point
                register_station(mac_from, ap);
            } else {
                // From access point to station
                ap = data.ap_list.search(mac_from);
                if (ap) register_station(mac_to, ap);
            }
        }
    }

    void clearAPresults() {
        data.ap_list.clear();
    }

    void clearSTresults() {
        data.st_list.clear();
    }

    void startAPsearch() {
        debugln("Scanning for access points (WiFi networks)");

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        WiFi.scanNetworks(true, true);
    }

    void startSTsearch() {
        debug("Scanning for stations (WiFi client devices) on ");
        debug(data.num_of_channels);
        debug(" different channels");
        if (data.timeout > 0) {
            debug(" in ");
            debug(data.timeout/1000);
            debug(" seconds");
        }
        debugln();
        debugln("Type 'stop' to stop the scan");

        uint8_t ch = 1;
        wifi_set_channel(ch);

        if ((data.channels >> (ch-1)) & 0x01) {
            printChannel(ch);
        } else {
            setNextChannel();
        }

        data.start_time     = millis();
        data.ch_update_time = data.start_time;

        wifi_set_promiscuous_rx_cb(station_sniffer);
        wifi_promiscuous_enable(true);
    }

    void stopAPsearch() {
        if (data.ap) {
            WiFi.scanDelete();
            data.ap = false;

            debugln("Stopped access point scan");
            debugln();

            printAPs();

            if (data.st) startSTsearch();
        }
    }

    void stopSTsearch() {
        if (data.st) {
            wifi_promiscuous_enable(false);
            data.st = false;

            debugln("Stopped station scan");
            debugln();

            printSTs();
        }
    }

    void stopAuthSearch() {
        if (data.auth) {
            wifi_promiscuous_enable(false);
            data.auth = false;

            debugln("Stopped beacon authentication scan");
            debugln();

            printSTs();
        }
    }

    void updateAPsearch() {
        if (data.ap && (WiFi.scanComplete() >= 0)) {
            int n = WiFi.scanComplete();

            for (int i = 0; i < n; ++i) {
                if (((data.channels >> (WiFi.channel(i)-1)) & 0x01)) {
                    data.ap_list.push(
                        WiFi.isHidden(i),
                        WiFi.SSID(i).c_str(),
                        WiFi.BSSID(i),
                        WiFi.RSSI(i),
                        WiFi.encryptionType(i),
                        WiFi.channel(i)
                        );
                }
            }

            stopAPsearch();
        }
    }

    void updateSTsearch() {
        if (!data.ap && data.st) {
            unsigned long current_time = millis();

            if (data.st_list.full()) {
                debugln("Station list full");
                stopSTsearch();
            } else if ((data.timeout > 0) && (current_time - data.start_time >= data.timeout)) {
                stopSTsearch();
            } else if ((data.ch_time > 0) && (current_time - data.ch_update_time >= data.ch_time)) {
                setNextChannel();
                data.ch_update_time = current_time;
            } else if (!data.silent && (current_time - data.output_time >= 1000)) {
                // print infos
                data.output_time = current_time;
            }
        }
    }

    void updateAuthSearch() {
        if (data.auth && (data.timeout > 0) && (millis() - data.start_time >= data.timeout)) {
            stopAuthSearch();
        }
    }

    // ===== PUBLIC ===== //
    void start(bool ap, bool st, unsigned long timeout, uint16_t channels, unsigned long ch_time, bool silent, bool retain) {
        { // Error check
            if (!ap && !st) {
                debugln("ERROR: Invalid scan mode");
                return;
            }

            if (st && (channels == 0)) {
                debugln("ERROR: No channels specified");
                return;
            }
        }

        stop();

        if (!retain) {
            if (ap) clearAPresults();
            if (st) clearSTresults();
        }

        uint8_t num_of_channels = 0;

        for (uint8_t i = 0; i<14; ++i) {
            num_of_channels += ((channels >> i) & 0x01);
        }

        if ((ch_time == 0) && (timeout > 0)) {
            ch_time = timeout/num_of_channels;
        }

        data.ap       = ap;
        data.st       = st;
        data.channels = channels;
        data.ch_time  = ch_time;
        data.timeout  = timeout;
        data.silent   = silent;

        unsigned long current_time = millis();

        data.num_of_channels = num_of_channels;
        data.start_time      = current_time;
        data.output_time     = current_time;
        data.pkt_time        = current_time;
        data.ch_update_time  = current_time;
        data.pkts_received   = 0;
        data.pkts_per_second = 0;

        if (ap) startAPsearch();
        else if (st) startSTsearch();
    }

    void startAuth(uint8_t* mac, unsigned long timeout, bool silent) {
        stop();

        data.auth       = true;
        data.timeout    = timeout;
        data.silent     = silent;
        data.start_time = millis();
        memcpy(data.bssid, mac, 5);

        debug("Scanning for authentications on ");
        debugln(strh::mac(data.bssid));
        debugln();

        // Reset authentications
        data.st_list.begin();

        while (data.st_list.available()) {
            Station* tmp = data.st_list.iterate();
            tmp->setAuth(0);
        }

        wifi_set_promiscuous_rx_cb(station_sniffer);
        wifi_promiscuous_enable(true);
    }

    void stop() {
        stopAPsearch();
        stopSTsearch();
        stopAuthSearch();
    }

    void printAPs(uint16_t channels, String ssids) {
        StringList ssid_list(ssids, ",");

        data.ap_list.print(channels, &ssid_list);
    }

    void printSTs(uint16_t channels, String ssids) {
        StringList ssid_list(ssids, ",");

        data.st_list.print(channels, &ssid_list);
    }

    void update() {
        updateAPsearch();
        updateSTsearch();
        updateAuthSearch();
    }

    AccessPointList& getAccessPoints() {
        return data.ap_list;
    }

    StationList& getStations() {
        return data.st_list;
    }
}