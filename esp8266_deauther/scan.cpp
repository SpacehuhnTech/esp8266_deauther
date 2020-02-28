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

#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}
typedef struct scan_data_t {
    bool          ap;
    bool          st;
    uint16_t      channels;
    unsigned long ch_time;
    unsigned long timeout;
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
                if (data.ch_time< 1000) {
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

    void station_sniffer(uint8_t* buf, uint16_t len) {
        if (!data.st) return;

        // drop frames that are too short to have a valid MAC header
        if (len < 28) return;

        // drop deauthentication and disassociation frames
        if ((buf[12] == 0xc0) || (buf[12] == 0xa0)) return;

        // drop beacon and probe response frames
        if ((buf[12] == 0x80) || (buf[12] == 0x50)) return;

        // only allow data frames
        // if(buf[12] != 0x08 && buf[12] != 0x88) return;

        uint8_t* mac_a = &buf[16]; // To (Receiver)
        uint8_t* mac_b = &buf[22]; // From (Transmitter)

        // drop frames with corrupted MAC addresses
        if (!mac::valid(mac_a) || !mac::valid(mac_b)) return;

        // frame from AP to station
        if (!mac::multicast(mac_a)) {
            AccessPoint* ap = data.ap_list.search(mac_b);
            if (ap) {
                if (data.st_list.registerPacket(mac_a, ap)) {
                    if (!data.silent) {
                        debug("Station ");
                        debug(strh::mac(mac_a));
                        debug(" in \"");
                        debug(ap->getSSID());
                        debugln('"');
                    }
                }
            }
        }
        // broadcast probe request from unassociated station
        else if ((buf[12] == 0x40) && (buf[12+25] > 0)) {
            if (data.st_list.registerPacket(mac_b, NULL)) {
                if (!data.silent) {
                    debug("Station ");
                    debug(strh::mac(mac_b));
                    debugln(' ');
                }
            }

            const char* ssid = (const char*)&buf[12+26];
            uint8_t     len  = buf[12+25];

            if ((ssid[0] != '\0') && data.st_list.addProbe(mac_b, ssid, len)) {
                if (!data.silent) {
                    debug("Probe \"");

                    for (uint8_t i = 0; i<len; ++i) {
                        debug(char(ssid[i]));
                    }
                    debugln("\"");
                }
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

    void updateAPsearch() {
        if (data.ap && (WiFi.scanComplete() >= 0)) {
            int n = WiFi.scanComplete();

            for (int i = 0; i < n; ++i) {
                if (((data.channels >> (WiFi.channel(i)-1)) & 0x01)) {
                    data.ap_list.push(
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

    void stop() {
        stopAPsearch();
        stopSTsearch();
    }

    void printAPs() {
        if (data.ap_list.size() == 0) {
            debugln("No access points (networks) found");
        } else {
            debug("Found ");
            debug(data.ap_list.size());
            debugln(" access points (networks):");

            debug(strh::right(3, "ID"));
            debug(' ');
            debug(strh::left(34, "SSID (Network Name)"));
            debug(' ');
            debug(strh::right(4, "RSSI"));
            debug(' ');
            debug(strh::left(4, "Mode"));
            debug(' ');
            debug(strh::right(2, "Ch"));
            debug(' ');
            debug(strh::left(17, "BSSID (MAC Addr.)"));
            debug(' ');
            debug(strh::left(8, "Vendor"));
            debugln();

            debugln("==============================================================================");

            data.ap_list.begin();
            int i = 0;

            while (data.ap_list.available()) {
                AccessPoint* h = data.ap_list.iterate();

                debug(strh::right(3, String(i)));
                debug(' ');
                debug(strh::left(34, h->getSSIDString()));
                debug(' ');
                debug(strh::right(4, String(h->getRSSI())));
                debug(' ');
                debug(strh::left(4, h->getEncryption()));
                debug(' ');
                debug(strh::right(2, String(h->getChannel())));
                debug(' ');
                debug(strh::left(17, h->getBSSIDString()));
                debug(' ');
                debug(strh::left(8, h->getVendor()));
                debugln();

                ++i;
            }

            debugln("================================================================================");
            debugln("Ch   = 2.4 GHz Channel");
            debugln("RSSI = Signal strengh");
            debugln("WPA* = WPA & WPA2 auto mode");
            debugln("WPA(2) Enterprise networks are recognized as Open");
            debugln("================================================================================");
        }
        debugln();
    }

    void printSTs() {
        if (data.st_list.size() == 0) {
            debugln("No stations (clients) found");
        } else {
            debug("Found ");
            debug(data.st_list.size());
            debugln(" stations (clients):");

            debug(strh::right(3, "ID"));
            debug(' ');
            debug(strh::left(17, "MAC-Address"));
            debug(' ');
            debug(strh::right(4, "Pkts"));
            debug(' ');
            debug(strh::left(8, "Vendor"));
            debug(' ');
            debug(strh::left(34, "AccessPoint-SSID"));
            debug(' ');
            debug(strh::left(17, "AccessPoint-BSSID"));
            debug(' ');
            debug(strh::left(34, "Probe-Requests"));
            debugln();

            debugln("===========================================================================================================================");

            int i = 0;
            data.st_list.begin();

            while (data.st_list.available()) {
                Station* h = data.st_list.iterate();

                debug(strh::right(3, String(i)));
                debug(' ');
                debug(h->getMACString());
                debug(' ');
                debug(strh::right(4, String(h->getPackets())));
                debug(' ');
                debug(strh::left(8, h->getVendor()));
                debug(' ');
                debug(strh::left(34, h->getSSIDString()));
                debug(' ');
                debug(strh::left(17, h->getBSSIDString()));
                debug(' ');

                h->getProbes().begin();
                bool first = true;

                while (h->getProbes().available()) {
                    if (!first) {
                        debugln();
                        debug("                                                                                         ");
                    }
                    debug(/*strh::left(32, */ '"' + h->getProbes().iterate() + '"');
                    first = false;
                }

                debugln();
                ++i;
            }

            debugln("===========================================================================================================================");
            debugln("Pkts = Recorded Packets");
            debugln("===========================================================================================================================");
        }
        debugln();
    }

    void print() {
        printAPs();
        printSTs();
    }

    void update() {
        updateAPsearch();
        updateSTsearch();
    }

    AccessPointList& getAccessPoints() {
        return data.ap_list;
    }

    StationList& getStations() {
        return data.st_list;
    }
}