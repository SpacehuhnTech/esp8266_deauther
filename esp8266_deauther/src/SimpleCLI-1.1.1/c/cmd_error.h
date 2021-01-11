/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef cmd_error_h
#define cmd_error_h

#include "cmd_error_types.h" // cmd_error

// ===== CMD Parse Error ===== //

// Constructors
cmd_error* cmd_error_create(int mode, cmd* command, arg* argument, word_node* data);
cmd_error* cmd_error_create_null_ptr(cmd* c);
cmd_error* cmd_error_create_empty_line(cmd* c);
cmd_error* cmd_error_create_parse_success(cmd* c);
cmd_error* cmd_error_create_not_found(cmd* c, word_node* cmd_name);
cmd_error* cmd_error_create_unknown_arg(cmd* c, word_node* arg_name);
cmd_error* cmd_error_create_missing_arg(cmd* c, arg* a);
cmd_error* cmd_error_create_unclosed_quote(cmd* c, arg* a, word_node* arg_value);

// Copy Constructors
cmd_error* cmd_error_copy(cmd_error* e);
cmd_error* cmd_error_copy_rec(cmd_error* e);

// Destructors
cmd_error* cmd_error_destroy(cmd_error* e);
cmd_error* cmd_error_destroy_rec(cmd_error* e);

// Push
cmd_error* cmd_error_push(cmd_error* l, cmd_error* e, int max_size);

#endif /* ifndef cmd_error_h */