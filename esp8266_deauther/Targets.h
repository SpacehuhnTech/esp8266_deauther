/*
   Based on Wireshark manufacturer database
   source: https://www.wireshark.org/tools/oui-lookup.html
   Wireshark is released under the GNU General Public License version 2
 */

#pragma once

#include <cstdint> // uint8_t
#include <cstddef> // NULL

class TargetList;
class Target;

typedef struct target_t target_t;

class TargetList {
    private:
        target_t* list_begin = NULL;
        target_t* list_end   = NULL;
        int list_size        = 0;

        target_t* h = NULL;

    public:
        ~TargetList();

        void push(const uint8_t* from, const uint8_t* to, const uint8_t ch);

        Target get(int i);
        Target next();

        bool available();
        int size();
};

class Target {
    private:
        target_t* ptr;

    public:
        Target(target_t* ptr);

        uint8_t* from() const;
        uint8_t* to() const;
        uint8_t ch() const;

        bool operator==(const Target& t) const;
};