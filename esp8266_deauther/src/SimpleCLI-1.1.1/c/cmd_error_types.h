/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef cmd_error_types_h
#define cmd_error_types_h

#include "cmd_types.h"    // cmd, arg
#include "parser_types.h" // word_node

#define CMD_NULL_PTR -2
#define CMD_EMPTY_LINE -1
#define CMD_PARSE_SUCCESS 0
#define CMD_NOT_FOUND 1
#define CMD_UNKOWN_ARG 2
#define CMD_MISSING_ARG 3
#define CMD_MISSING_ARG_VALUE 4
#define CMD_UNCLOSED_QUOTE 5

typedef struct cmd_error {
    int               mode;
    cmd             * command;
    arg             * argument;
    char            * data;
    struct cmd_error* next;
} cmd_error;

#endif /* ifndef cmd_error_types_h */