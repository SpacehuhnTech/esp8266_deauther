/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef CommandError_h
#define CommandError_h

#include "StringCLI.h"
#include "Command.h"             // Command

extern "C" {
  #include "c/cmd_error_types.h" // cmd_error
}

#define COMMAND_ERROR_TEMPORARY false
#define COMMAND_ERROR_PERSISTENT true

enum class CommandErrorType { NULL_POINTER, EMPTY_LINE, PARSE_SUCCESSFUL,
                              COMMAND_NOT_FOUND, UNKNOWN_ARGUMENT, MISSING_ARGUMENT,
                              MISSING_ARGUMENT_VALUE, UNCLOSED_QUOTE };

class CommandError {
    private:
        cmd_error* errorPointer;
        bool persistent;

    public:
        CommandError(cmd_error* errorPointer = NULL, bool persistent = COMMAND_ERROR_PERSISTENT);
        CommandError(const CommandError& e);
        CommandError(CommandError&& e);
        ~CommandError();

        CommandError& operator=(const CommandError& e);
        CommandError& operator=(CommandError&& e);

        bool operator==(const CommandError& e) const;
        bool operator!=(const CommandError& e) const;

        bool operator>(const CommandError& e) const;
        bool operator<(const CommandError& e) const;

        bool operator>=(const CommandError& e) const;
        bool operator<=(const CommandError& e) const;

        operator bool() const;

        bool hasCommand() const;
        bool hasArgument() const;
        bool hasData() const;

        bool hasCmd() const;
        bool hasArg() const;

        CommandErrorType getType() const;
        Command getCommand() const;
        Argument getArgument() const;
        String getData() const;
        String getMessage() const;

        Command getCmd() const;
        Argument getArg() const;
        String getMsg() const;

        String toString() const;
        void toString(String& s) const;

        cmd_error* getPtr();
};

#endif /* ifndef CommandError_h */