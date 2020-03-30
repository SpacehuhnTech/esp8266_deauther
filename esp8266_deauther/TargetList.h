/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "config.h"

#ifndef MAX_TARGETS
#define MAX_TARGETS 0
#endif // ifndef MAX_ALIAS

class TargetList;
class Target;

class Target {
    private:
        uint8_t sender[6];
        uint8_t receiver[6];
        uint8_t ch;
        Target* next;

    public:
        Target(const uint8_t* sender, const uint8_t* receiver, uint8_t ch);

        const uint8_t* getSender() const;
        const uint8_t* getReceiver() const;
        uint8_t getCh() const;

        Target* getNext();

        void setNext(Target* next);
};

class TargetList {
    private:
        Target* list_begin = nullptr;
        Target* list_end   = nullptr;

        int list_size     = 0;
        int list_max_size = 0;

        Target* list_h = nullptr;
        int list_pos   = 0;

        int compare(const Target* a, const Target* b) const;

    public:
        TargetList(int max = MAX_TARGETS);
        ~TargetList();

        void moveFrom(TargetList& t);

        bool push(const uint8_t* sender, const uint8_t* receiver, const uint8_t ch);

        Target* get(int i);

        void begin();
        Target* iterate();

        bool available() const;
        int size() const;
        bool full() const;

        void clear();
};