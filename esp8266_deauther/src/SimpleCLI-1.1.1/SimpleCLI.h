/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef SimpleCLI_h
#define SimpleCLI_h

#include "CommandError.h" // CommandError, Command, Argument

#define SIMPLECLI_VERSION "1.1.1"
#define SIMPLECLI_VERSION_MAJOR 1
#define SIMPLECLI_VERSION_MINOR 1
#define SIMPLECLI_VERSION_REVISION 1

class SimpleCLI {
    private:
        bool caseSensetive { false };
        bool pauseParsing { false };

        cmd* cmdList { NULL };          // List of accessible commands
        cmd* cmdQueue { NULL };         // Queue with parsed commands the user has typed in
        cmd_error* errorQueue { NULL }; // Queue with parser errors

        int commandQueueSize;
        int errorQueueSize;

        void (* onError)(cmd_error* e) = NULL;

        cmd* getNextCmd(cmd* begin, const char* name, size_t name_len);
        void parseLine(const char* input, size_t input_len);

        void addCmd(Command& c);

    public:
        SimpleCLI(int commandQueueSize = 10, int errorQueueSize = 10);
        ~SimpleCLI();

        void pause();
        void unpause();

        void parse(const String& input);
        void parse(const char* input);
        void parse(const char* input, size_t input_len);

        bool available() const;
        bool errored() const;
        bool paused() const;

        int countCmdQueue() const;
        int countErrorQueue() const;

        Command getCmd();
        Command getCmd(String name);
        Command getCmd(const char* name);

        Command getCommand();
        Command getCommand(String name);
        Command getCommand(const char* name);

        CommandError getError();

        Command addCmd(const char* name, void (* callback)(cmd* c)          = NULL);
        Command addBoundlessCmd(const char* name, void (* callback)(cmd* c) = NULL);
        Command addSingleArgCmd(const char* name, void (* callback)(cmd* c) = NULL);

        Command addCommand(const char* name, void (* callback)(cmd* c)               = NULL);
        Command addBoundlessCommand(const char* name, void (* callback)(cmd* c)      = NULL);
        Command addSingleArgumentCommand(const char* name, void (* callback)(cmd* c) = NULL);

        String toString(bool descriptions          = true) const;
        void toString(String& s, bool descriptions = true) const;

        void setCaseSensetive(bool caseSensetive = true);
        void setOnError(void (* onError)(cmd_error* e));
        void setErrorCallback(void (* onError)(cmd_error* e));
};

#endif // ifndef SimpleCLI_h