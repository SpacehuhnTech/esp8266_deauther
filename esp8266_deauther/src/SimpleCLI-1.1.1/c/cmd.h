/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Soruce: github.com/spacehuhn/SimpleCLI
 */

#ifndef cmd_h
#define cmd_h

#include "cmd_types.h"       // cmd, cmd_parse_error
#include "parser_types.h"    // line_node, word_list, word_node, ...
#include "cmd_error_types.h" // cmd_error

// ===== CMD ===== //

// Constructors
cmd* cmd_create(const char* name, unsigned int mode);
cmd* cmd_create_default(const char* name);
cmd* cmd_create_boundless(const char* name);
cmd* cmd_create_single(const char* name);

// Copy & Move Constructors
cmd* cmd_copy(cmd* c);
cmd* cmd_copy_rec(cmd* c);
cmd* cmd_move(cmd* c);
cmd* cmd_move_rec(cmd* c);

// Destructors
cmd* cmd_destroy(cmd* c);
cmd* cmd_destroy_rec(cmd* c);

// Push CMD and Push Arg
cmd* cmd_push(cmd* l, cmd* c, int max_size);
cmd* cmd_add_arg(cmd* c, arg* a);

// Reset CMD
void cmd_reset(cmd* c);
void cmd_reset_rec(cmd* c);

// Comparisons
int cmd_name_equals(cmd* c, const char* name, size_t name_len, int case_sensetive);
int cmd_equals(cmd* a, cmd* b, int case_sensetive);

// Parser
cmd_error* cmd_parse(cmd* c, line_node* n);

// Getter
const char* cmd_get_description(cmd* c);

// Setter
void cmd_set_description(cmd* c, const char* description);

#endif /* ifndef cmd_h */