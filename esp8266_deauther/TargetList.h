/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "config.h"

class TargetList;
class Target;

class Target {
    private:
        uint8_t from[6];
        uint8_t to[6];
        uint8_t ch;
        Target* next;

    public:
        Target(const uint8_t* from, const uint8_t* to, uint8_t ch);

        const uint8_t* getFrom() const;
        const uint8_t* getTo() const;
        uint8_t getCh() const;

        Target* getNext();

        void setNext(Target* next);

        bool operator==(const Target& t) const;
        bool operator<(const Target& t) const;
        bool operator>(const Target& t) const;
};

class TargetList {
    private:
        Target* list_begin = NULL;
        Target* list_end   = NULL;

        int list_size     = 0;
        int list_max_size = 0;

        Target* list_h = NULL;
        int list_pos   = 0;

    public:
        TargetList(int max = MAX_TARGETS);
        ~TargetList();

        void moveFrom(TargetList& t);

        bool push(const uint8_t* from, const uint8_t* to, const uint8_t ch);

        Target* get(int i);

        void begin();
        Target* iterate();

        bool available() const;
        int size() const;
        bool full() const;

        void clear();
};