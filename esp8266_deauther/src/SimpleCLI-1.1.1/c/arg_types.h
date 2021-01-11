/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef arg_types_h
#define arg_types_h

#define ARG_DEFAULT 0
#define ARG_POS 1
#define ARG_FLAG 2

#define ARG_OPT 0
#define ARG_REQ 1

#define ARG_UNSET 0
#define ARG_SET 1

#define ARG_NAME_UNEQUALS 0
#define ARG_NAME_EQUALS 1

#define ARG_VALUE_FAIL 0
#define ARG_VALUE_SUCCESS 1

typedef struct arg {
    const char * name;
    const char * default_val;
    char       * val;
    unsigned int mode : 2;
    unsigned int req  : 1;
    unsigned int set  : 1;
    struct arg * next;
} arg;

#endif /* ifndef arg_types_h */