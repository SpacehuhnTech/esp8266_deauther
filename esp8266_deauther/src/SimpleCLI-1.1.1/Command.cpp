/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include "Command.h"

extern "C" {
#include "c/cmd.h"
#include "c/arg.h"
}

Command::Command(cmd* cmdPointer, bool persistent) : cmdPointer(cmdPointer), persistent(persistent) {
    if (!persistent) this->cmdPointer = cmd_copy(cmdPointer);
}

Command::Command(const Command& c) {
    persistent = c.persistent;
    cmdPointer = c.cmdPointer;
    if (!persistent) cmdPointer = cmd_copy(c.cmdPointer);
}

Command::Command(Command&& c) {
    persistent   = c.persistent;
    cmdPointer   = c.cmdPointer;
    c.cmdPointer = NULL;
}

Command::~Command() {
    if (!persistent) cmd_destroy(cmdPointer);
}

Command& Command::operator=(const Command& c) {
    persistent = c.persistent;
    cmdPointer = c.cmdPointer;
    if (!persistent) cmdPointer = cmd_copy(c.cmdPointer);

    return *this;
}

Command& Command::operator=(Command&& c) {
    persistent   = c.persistent;
    cmdPointer   = c.cmdPointer;
    c.cmdPointer = NULL;

    return *this;
}

bool Command::operator==(const Command& c) const {
    return equals(c);
}

bool Command::operator!=(const Command& c) const {
    return !equals(c);
}

Command::operator bool() const {
    return cmdPointer;
}

bool Command::setCaseSensetive(bool caseSensetive) {
    if (cmdPointer) {
        cmdPointer->case_sensetive = caseSensetive;
        return true;
    }
    return false;
}

bool Command::setCallback(void (* callback)(cmd* c)) {
    if (cmdPointer && callback) {
        cmdPointer->callback = callback;
        return true;
    }
    return false;
}

void Command::setDescription(const char* description) {
    cmd_set_description(cmdPointer, description);
}

Argument Command::addArg(const char* name, const char* defaultValue) {
    if (cmdPointer && (cmdPointer->mode == CMD_DEFAULT)) {
        arg* a = arg_create_opt(name, defaultValue);

        cmd_add_arg(cmdPointer, a);
        return Argument(a);
    }
    return Argument();
}

Argument Command::addArg(const char* name) {
    if (cmdPointer && (cmdPointer->mode == CMD_DEFAULT)) {
        arg* a = arg_create_req(name);

        cmd_add_arg(cmdPointer, a);
        return Argument(a);
    }
    return Argument();
}

Argument Command::addPosArg(const char* name, const char* defaultValue) {
    if (cmdPointer && (cmdPointer->mode == CMD_DEFAULT)) {
        arg* a = arg_create_opt_positional(name, defaultValue);

        cmd_add_arg(cmdPointer, a);
        return Argument(a);
    }
    return Argument();
}

Argument Command::addPosArg(const char* name) {
    if (cmdPointer && (cmdPointer->mode == CMD_DEFAULT)) {
        arg* a = arg_create_req_positional(name);

        cmd_add_arg(cmdPointer, a);
        return Argument(a);
    }
    return Argument();
}

Argument Command::addFlagArg(const char* name, const char* defaultValue) {
    if (cmdPointer && (cmdPointer->mode == CMD_DEFAULT)) {
        arg* a = arg_create_flag(name, defaultValue);

        cmd_add_arg(cmdPointer, a);
        return Argument(a);
    }
    return Argument();
}

Argument Command::addArgument(const char* name, const char* defaultValue) {
    return addArg(name, defaultValue);
}

Argument Command::addArgument(const char* name) {
    return addArg(name);
}

Argument Command::addPositionalArgument(const char* name, const char* defaultValue) {
    return addPosArg(name, defaultValue);
}

Argument Command::addPositionalArgument(const char* name) {
    return addPosArg(name);
}

Argument Command::addFlagArgument(const char* name, const char* defaultValue) {
    return addFlagArg(name, defaultValue);
}

bool Command::equals(String name) const {
    return equals(name.c_str());
}

bool Command::equals(const char* name) const {
    if (cmdPointer && name) return cmd_name_equals(cmdPointer, name, strlen(name), cmdPointer->case_sensetive) == CMD_NAME_EQUALS;
    return false;
}

bool Command::equals(const Command& c) const {
    if (cmdPointer && c.cmdPointer) return cmd_equals(cmdPointer, c.cmdPointer, cmdPointer->case_sensetive) == CMD_NAME_EQUALS;
    return false;
}

String Command::getName() const {
    if (cmdPointer) {
        return String(cmdPointer->name);
    }
    return String();
}

int Command::countArgs() const {
    int i = 0;

    if (cmdPointer) {
        arg* h = cmdPointer->arg_list;

        while (h) {
            ++i;
            h = h->next;
        }
    }
    return i;
}

Argument Command::getArgument(int i) const {
    if (!cmdPointer) return Argument();

    arg* h = cmdPointer->arg_list;
    int  j = 0;

    while (j < i && h) {
        h = h->next;
        ++j;
    }

    return Argument(j == i ? h : NULL);
}

Argument Command::getArgument(const char* name) const {
    if (!cmdPointer || !name) return Argument();

    arg* h = cmdPointer->arg_list;
    int  j = 0;

    while (h) {
        if (arg_name_equals(h, name, strlen(name), cmdPointer->case_sensetive) == ARG_NAME_EQUALS) return h;
        h = h->next;
        ++j;
    }

    return Argument();
}

Argument Command::getArgument(String name) const {
    return getArgument(name.c_str());
}

Argument Command::getArgument(const Argument& a) const {
    return getArgument(a.argPointer ? a.argPointer->name : NULL);
}

Argument Command::getArg(int i) const {
    return getArgument(i);
}

Argument Command::getArg(const char* name) const {
    return getArgument(name);
}

Argument Command::getArg(String name) const {
    return getArgument(name);
}

Argument Command::getArg(const Argument& a) const {
    return getArgument(a);
}

String Command::toString(bool description) const {
    String s;

    toString(s, description);
    return s;
}

CommandType Command::getType() const {
    if (cmdPointer) {
        switch (cmdPointer->mode) {
            case CMD_DEFAULT:
                return CommandType::NORMAL;
            case CMD_BOUNDLESS:
                return CommandType::BOUNDLESS;
            case CMD_SINGLE:
                return CommandType::SINGLE;
        }
    }
    return CommandType::NORMAL;
}

bool Command::hasDescription() const {
    return cmdPointer && cmdPointer->description;
}

String Command::getDescription() const {
    return String(cmd_get_description(cmdPointer));
}

void Command::toString(String& s, bool description) const {
    if (cmdPointer) {
        s += String(cmdPointer->name);

        if (cmdPointer->mode == CMD_BOUNDLESS) {
            s += ' ';
            s += "<value> <value> ...";
        } else if (cmdPointer->mode == CMD_SINGLE) {
            s += ' ';
            s += "<...>";
        } else  {
            arg* h = cmdPointer->arg_list;

            while (h) {
                s += ' ';
                Argument(h).toString(s);

                h = h->next;
            }
        }

        if (description && hasDescription()) {
            s += "\r\n" + getDescription();
        }
    }
}

void Command::run() const {
    if (cmdPointer && cmdPointer->callback) cmdPointer->callback(cmdPointer);
}

cmd* Command::getPtr() {
    return cmdPointer;
}