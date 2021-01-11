/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef Argument_h
#define Argument_h

#include "StringCLI.h"

extern "C" {
#include "c/arg_types.h" // arg
}

#define ARGUMENT_TEMPORARY false
#define ARGUMENT_PERSISTENT true

enum class ArgumentType { NORMAL, POSITIONAL, FLAG };

class Argument {
    friend class Command;
    friend class SimpleCLI;

    private:
        arg* argPointer;
        bool persistent;

    public:
        Argument(arg* argPointer = NULL, bool persistent = ARGUMENT_PERSISTENT);
        Argument(const Argument& a);
        Argument(Argument&& a);

        ~Argument();

        Argument& operator=(const Argument& a);
        Argument& operator=(Argument&& a);

        bool operator==(const Argument& a) const;
        bool operator!=(const Argument& a) const;

        operator bool() const;

        bool isSet() const;
        bool isRequired() const;
        bool isOptional() const;
        bool hasDefaultValue() const;

        bool isReq() const;
        bool isOpt() const;

        String getName() const;
        String getValue() const;

        ArgumentType getType() const;

        String toString() const;
        void toString(String& s) const;

        bool equals(String name, bool caseSensetive       = false) const;
        bool equals(const char* name, bool caseSensetive  = false) const;
        bool equals(const Argument& a, bool caseSensetive = false) const;

        arg* getPtr();
};

#endif /* ifndef Argument_h */