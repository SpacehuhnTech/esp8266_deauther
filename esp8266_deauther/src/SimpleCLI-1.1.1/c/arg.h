/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef arg_h
#define arg_h

#include "arg_types.h" // arg
#include <stddef.h>    // size_t

// ===== Arg ===== //

// Constructors
arg* arg_create(const char* name, const char* default_val, unsigned int mode, unsigned int req);
arg* arg_create_opt(const char* name, const char* default_val);
arg* arg_create_req(const char* name);
arg* arg_create_opt_positional(const char* name, const char* default_value);
arg* arg_create_req_positional(const char* name);
arg* arg_create_flag(const char* name, const char* default_value);

// Copy & Move Constructors
arg* arg_copy(arg* a);
arg* arg_copy_rec(arg* a);
arg* arg_move(arg* a);
arg* arg_move_rec(arg* a);

// Destructors
arg* arg_destroy(arg* a);
arg* arg_destroy_rec(arg* a);

// Reset
void arg_reset(arg* a);
void arg_reset_rec(arg* a);

// Comparisons
int arg_name_equals(arg* a, const char* name, size_t name_len, int case_sensetive);
int arg_equals(arg* a, arg* b, int case_sensetive);

// Getter
const char* arg_get_value(arg* a);

// Setter
int arg_set_value(arg* a, const char* val, size_t val_size);

#endif /* ifndef arg_h */