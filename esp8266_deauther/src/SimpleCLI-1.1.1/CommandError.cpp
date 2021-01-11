/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include "CommandError.h"

extern "C" {
  #include "c/cmd_error.h" // cmd_error_create, cmd_error_destroy
}

CommandError::CommandError(cmd_error* errorPointer, bool persistent) : errorPointer(errorPointer), persistent(persistent) {
    if (!persistent) this->errorPointer = cmd_error_copy(errorPointer);
}

CommandError::CommandError(const CommandError& e) {
    persistent   = e.persistent;
    errorPointer = e.errorPointer;
    if (!persistent) errorPointer = cmd_error_copy(errorPointer);
}

CommandError::CommandError(CommandError&& e) {
    persistent     = e.persistent;
    errorPointer   = e.errorPointer;
    e.errorPointer = NULL;
}

CommandError::~CommandError() {
    if (!persistent) cmd_error_destroy(errorPointer);
}

CommandError& CommandError::operator=(const CommandError& e) {
    persistent   = e.persistent;
    errorPointer = e.errorPointer;
    if (!persistent) errorPointer = cmd_error_copy(errorPointer);
    return *this;
}

CommandError& CommandError::operator=(CommandError&& e) {
    persistent     = e.persistent;
    errorPointer   = e.errorPointer;
    e.errorPointer = NULL;
    return *this;
}

bool CommandError::operator==(const CommandError& e) const {
    return errorPointer == e.errorPointer ||
           (errorPointer && e.errorPointer &&
            errorPointer->mode == e.errorPointer->mode &&
            errorPointer->command == e.errorPointer->command &&
            errorPointer->argument == e.errorPointer->argument &&
            errorPointer->data == e.errorPointer->data);
}

bool CommandError::operator!=(const CommandError& e) const {
    return !(*this == e);
}

bool CommandError::operator>(const CommandError& e) const {
    return errorPointer && e.errorPointer && errorPointer->mode > e.errorPointer->mode;
}

bool CommandError::operator<(const CommandError& e) const {
    return errorPointer && e.errorPointer && errorPointer->mode < e.errorPointer->mode;
}

bool CommandError::operator>=(const CommandError& e) const {
    return errorPointer && e.errorPointer && errorPointer->mode >= e.errorPointer->mode;
}

bool CommandError::operator<=(const CommandError& e) const {
    return errorPointer && e.errorPointer && errorPointer->mode <= e.errorPointer->mode;
}

CommandError::operator bool() const {
    return errorPointer && errorPointer->mode != CMD_PARSE_SUCCESS;
}

bool CommandError::hasCommand() const {
    return errorPointer && errorPointer->command;
}

bool CommandError::hasArgument() const {
    return errorPointer && errorPointer->argument;
}

bool CommandError::hasData() const {
    return errorPointer && errorPointer->data;
}

CommandErrorType CommandError::getType() const {
    if (errorPointer) {
        switch (errorPointer->mode) {
            case CMD_NULL_PTR: return CommandErrorType::NULL_POINTER;
            case CMD_EMPTY_LINE: return CommandErrorType::EMPTY_LINE;
            case CMD_PARSE_SUCCESS: return CommandErrorType::PARSE_SUCCESSFUL;
            case CMD_NOT_FOUND: return CommandErrorType::COMMAND_NOT_FOUND;
            case CMD_UNKOWN_ARG: return CommandErrorType::UNKNOWN_ARGUMENT;
            case CMD_MISSING_ARG: return CommandErrorType::MISSING_ARGUMENT;
            case CMD_MISSING_ARG_VALUE: return CommandErrorType::MISSING_ARGUMENT_VALUE;
            case CMD_UNCLOSED_QUOTE: return CommandErrorType::UNCLOSED_QUOTE;
        }
    }
    return CommandErrorType::PARSE_SUCCESSFUL;
}

bool CommandError::hasCmd() const {
    return hasCommand();
}

bool CommandError::hasArg() const {
    return hasArgument();
}

Command CommandError::getCommand() const {
    return Command(errorPointer ? errorPointer->command : NULL);
}

Argument CommandError::getArgument() const {
    return Argument(errorPointer ? errorPointer->argument : NULL);
}

String CommandError::getData() const {
    if (!errorPointer || !errorPointer->data) return String();
    return String(errorPointer->data);
}

String CommandError::getMessage() const {
    if (errorPointer) {
        switch (errorPointer->mode) {
            case CMD_NULL_PTR: return String(F("NULL Pointer"));
            case CMD_EMPTY_LINE: return String(F("Empty input"));
            case CMD_PARSE_SUCCESS: return String(F("No error"));
            case CMD_NOT_FOUND: return String(F("Command not found"));
            case CMD_UNKOWN_ARG: return String(F("Unknown argument"));
            case CMD_MISSING_ARG: return String(F("Missing argument"));
            case CMD_MISSING_ARG_VALUE: return String(F("Missing argument value"));
            case CMD_UNCLOSED_QUOTE: return String(F("Unclosed quote"));
        }
    }
    return String();
}

Command CommandError::getCmd() const {
    return getCommand();
}

Argument CommandError::getArg() const {
    return getArgument();
}

String CommandError::getMsg() const {
    return getMessage();
}

String CommandError::toString() const {
    String s;

    toString(s);
    return s;
}

void CommandError::toString(String& s) const {
    s += getMessage();
    if (hasCommand()) s += String(F(" at command '")) + getCommand().getName() + String(F("'"));
    if (hasArgument()) s += String(F(" at argument '")) + getArgument().toString() + String(F("'"));
    if (hasData()) s += String(F(" at '")) + getData() + String(F("'"));
}

cmd_error* CommandError::getPtr() {
    return errorPointer;
}