/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include "Argument.h"

extern "C" {
#include "c/arg.h"
}

Argument::Argument(arg* argPointer, bool persistent) : argPointer(argPointer), persistent(persistent) {
    if (!persistent) argPointer = arg_copy(argPointer);
}

Argument::Argument(const Argument& a) {
    argPointer = a.argPointer;
    persistent = a.persistent;
    if (!persistent) argPointer = arg_copy(argPointer);
}

Argument::Argument(Argument&& a) {
    argPointer   = a.argPointer;
    persistent   = a.persistent;
    a.argPointer = NULL;
}

Argument::~Argument() {
    if (!persistent) arg_destroy(argPointer);
}

Argument& Argument::operator=(const Argument& a) {
    argPointer = a.argPointer;
    persistent = a.persistent;
    if (!persistent) argPointer = arg_copy(argPointer);

    return *this;
}

Argument& Argument::operator=(Argument&& a) {
    argPointer   = a.argPointer;
    persistent   = a.persistent;
    a.argPointer = NULL;

    return *this;
}

bool Argument::operator==(const Argument& a) const {
    return equals(a);
}

bool Argument::operator!=(const Argument& a) const {
    return !equals(a);
}

Argument::operator bool() const {
    return argPointer;
}

bool Argument::isSet() const {
    return argPointer && argPointer->set == ARG_SET;
}

bool Argument::isRequired() const {
    return argPointer && argPointer->req == ARG_REQ;
}

bool Argument::isOptional() const {
    return !isRequired();
}

bool Argument::hasDefaultValue() const {
    return isOptional();
}

bool Argument::isReq() const {
    return isRequired();
}

bool Argument::isOpt() const {
    return isOptional();
}

String Argument::getName() const {
    if (argPointer) return String(argPointer->name);
    return String();
}

String Argument::getValue() const {
    if (argPointer) return String(arg_get_value(argPointer));
    return String();
}

ArgumentType Argument::getType() const {
    if (argPointer) {
        if (argPointer->mode == ARG_DEFAULT) return ArgumentType::NORMAL;
        if (argPointer->mode == ARG_POS) return ArgumentType::POSITIONAL;
        if (argPointer->mode == ARG_FLAG) return ArgumentType::FLAG;
    }
    return ArgumentType::NORMAL;
}

String Argument::toString() const {
    String s;

    toString(s);
    return s;
}

void Argument::toString(String& s) const {
    if (isOptional()) s += '[';

    String n = getName();
    String v = getValue();

    switch (getType()) {
        case ArgumentType::NORMAL:
        case ArgumentType::POSITIONAL:
            s += '-';
            s += n;
            s += ' ';
            s += '<';
            s += v.length() > 0 ? v : "value";
            s += '>';
            break;

        case ArgumentType::FLAG:
            s += '-';
            s += n;
            break;
    }

    if (isOptional()) s += ']';
}

bool Argument::equals(String name, bool caseSensetive) const {
    return equals(name.c_str(), caseSensetive);
}

bool Argument::equals(const char* name, bool caseSensetive) const {
    return argPointer && name && arg_name_equals(argPointer, name, strlen(name), caseSensetive);
}

bool Argument::equals(const Argument& a, bool caseSensetive) const {
    return argPointer && a.argPointer && arg_equals(argPointer, a.argPointer, caseSensetive);
}

arg* Argument::getPtr() {
    return argPointer;
}