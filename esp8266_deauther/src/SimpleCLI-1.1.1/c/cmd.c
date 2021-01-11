/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Soruce: github.com/spacehuhn/SimpleCLI
 */

#include <stdlib.h>     // malloc
#include <string.h>     // strlen

#include "comparator.h" // compare

#include "cmd.h"

#include "cmd_error.h"
#include "arg.h"

// ===== CMD ===== //

// CMD Constructors
cmd* cmd_create(const char* name, unsigned int mode) {
    if (!name) return NULL;

    cmd* c = (cmd*)malloc(sizeof(cmd));

    c->name           = name;
    c->mode           = mode % 3;
    c->arg_list       = NULL;
    c->case_sensetive = COMPARE_CASE_INSENSETIVE;
    c->callback       = NULL;
    c->description    = NULL;
    c->next           = NULL;

    return c;
}

cmd* cmd_create_default(const char* name) {
    return cmd_create(name, CMD_DEFAULT);
}

cmd* cmd_create_boundless(const char* name) {
    return cmd_create(name, CMD_BOUNDLESS);
}

cmd* cmd_create_single(const char* name) {
    cmd* c = cmd_create(name, CMD_SINGLE);

    c->arg_list = arg_create_opt_positional(NULL, NULL);
    return c;
}

// Copy & Move Constructors
cmd* cmd_copy(cmd* c) {
    if (!c) return NULL;

    cmd* nc = (cmd*)malloc(sizeof(cmd));

    nc->name           = c->name;
    nc->mode           = c->mode;
    nc->arg_list       = arg_copy_rec(c->arg_list);
    nc->case_sensetive = c->case_sensetive;
    nc->callback       = c->callback;
    nc->description    = c->description;
    nc->next           = NULL;

    return nc;
}

cmd* cmd_copy_rec(cmd* c) {
    if (!c) return NULL;

    cmd* nc = cmd_copy(c);

    nc->next = cmd_copy_rec(c->next);

    return nc;
}

cmd* cmd_move(cmd* c) {
    if (!c) return NULL;

    cmd* nc = (cmd*)malloc(sizeof(cmd));

    nc->name           = c->name;
    nc->mode           = c->mode;
    nc->arg_list       = arg_move_rec(c->arg_list);
    nc->case_sensetive = c->case_sensetive;
    nc->callback       = c->callback;
    nc->description    = c->description;
    nc->next           = NULL;

    return nc;
}

cmd* cmd_move_rec(cmd* c) {
    if (!c) return NULL;

    cmd* nc = cmd_move(c);

    nc->next = cmd_move_rec(c->next);

    return nc;
}

// Destructors
cmd* cmd_destroy(cmd* c) {
    if (c) {
        arg_destroy_rec(c->arg_list);
        free(c);
    }
    return NULL;
}

cmd* cmd_destroy_rec(cmd* c) {
    if (c) {
        cmd_destroy_rec(c->next);
        cmd_destroy(c);
    }
    return NULL;
}

// Push CMD and Push Arg
cmd* cmd_push(cmd* l, cmd* c, int max_size) {
    if (max_size < 1) {
        cmd_destroy_rec(l);
        cmd_destroy(c);
        return NULL;
    }

    if (!l) return c;

    cmd* h = l;
    int  i = 1;

    while (h->next) {
        h = h->next;
        ++i;
    }

    h->next = c;

    // Remove first element if list is too big
    if (i > max_size) {
        cmd* ptr = l;
        l = l->next;
        cmd_destroy(ptr);
    }

    return l;
}

cmd* cmd_add_arg(cmd* c, arg* a) {
    if (c && a) {
        arg* h = c->arg_list;

        if (!h) {
            c->arg_list = a;
        } else {
            while (h->next) h = h->next;
            h->next = a;
        }

        a->next = NULL;
    }
    return c;
}

// Reset CMD
void cmd_reset(cmd* c) {
    if (c) {
        if (c->mode == CMD_BOUNDLESS) {
            arg_destroy_rec(c->arg_list);
            c->arg_list = NULL;
        } else {
            arg_reset_rec(c->arg_list);
        }
    }
}

void cmd_reset_rec(cmd* c) {
    if (c) {
        cmd_reset(c);
        cmd_reset_rec(c->next);
    }
}

// Comparisons
int cmd_name_equals(cmd* c, const char* name, size_t name_len, int case_sensetive) {
    if (!c || !name) return CMD_NAME_UNEQUALS;
    if (c->name == name) return CMD_NAME_EQUALS;
    return compare(name, name_len, c->name, case_sensetive) == COMPARE_EQUAL ? CMD_NAME_EQUALS : CMD_NAME_UNEQUALS;
}

int cmd_equals(cmd* a, cmd* b, int case_sensetive) {
    if (!a || !b) return CMD_NAME_UNEQUALS;
    if (a == b) return CMD_NAME_EQUALS;
    return cmd_name_equals(a, b->name, strlen(b->name), case_sensetive);
}

// Parser
cmd_error* cmd_parse(cmd* c, line_node* n) {
    if (!c || !n) return cmd_error_create_null_ptr(c);
    if (!n->words || (n->words->size == 0) || !n->words->first) return cmd_error_create_empty_line(c);

    word_list* wl        = n->words;
    word_node* cmd_name  = wl->first;
    word_node* first_arg = cmd_name->next;

    // Check if name equals command name
    if (compare(cmd_name->str, cmd_name->len, c->name, c->case_sensetive) == COMPARE_UNEQUAL) return cmd_error_create_not_found(c, cmd_name);

    // When command boundless, set all words as anonymous args
    if (c->mode == CMD_BOUNDLESS) {
        // Delete all old args
        arg_destroy_rec(c->arg_list);
        c->arg_list = NULL;

        // Fill command with an anonymous arg for each word
        word_node* w = first_arg;

        while (w) {
            arg* a = arg_create_req_positional(NULL);
            arg_set_value(a, w->str, w->len);
            cmd_add_arg(c, a);
            w = w->next;
        }

        return cmd_error_create_parse_success(c);
    }

    // When command single-arg, set full string as first arg
    if (c->mode == CMD_SINGLE) {
        if (!c->arg_list) c->arg_list = arg_create_opt_positional(NULL, NULL);
        if (wl->size > 1) arg_set_value(c->arg_list, first_arg->str, n->len - cmd_name->len - 1);
        return cmd_error_create_parse_success(c);
    }

    // Go through all words and try to find a matching arg
    word_node* w  = first_arg;
    word_node* nw = w ? w->next : NULL;

    while (w) {
        // Look for arg which matches the word name
        arg* a      = c->arg_list;
        char prefix = w->str[0];

        while (a) {
            if (a->set == ARG_UNSET) {
                if ((prefix != '-') && (a->mode == ARG_POS)) break;
                if ((prefix == '-') && (compare(&w->str[1], w->len - 1, a->name, c->case_sensetive) == COMPARE_EQUAL)) break;
            }
            a = a->next;
        }

        // No mathing arg found
        if (!a) return cmd_error_create_unknown_arg(c, w);

        switch (a->mode) {
            // Anonym, Template Arg -> value = value
            case ARG_POS:
                if (prefix != '-') {
                    arg_set_value(a, w->str, w->len);
                    break;
                }

            // Default Arg -> value in next word
            case ARG_DEFAULT:
                if (!nw) return cmd_error_create_missing_arg(c, a);
                if (arg_set_value(a, nw->str, nw->len) == ARG_VALUE_FAIL) return cmd_error_create_unclosed_quote(c, a, nw);
                w  = w->next;
                nw = w ? w->next : NULL;
                break;

            // Empty Arg -> no value
            case ARG_FLAG:
                arg_set_value(a, NULL, 0);
                break;
        }

        // Next word
        if (w) {
            w  = w->next;
            nw = w ? w->next : NULL;
        }
    }

    // Check if all required args have been set
    arg* a = c->arg_list;

    while (a) {
        if (a->req && !a->set) {
            return cmd_error_create_missing_arg(c, a);
        }
        a = a->next;
    }

    return cmd_error_create_parse_success(c);
}

// Getter
const char* cmd_get_description(cmd* c) {
    if (!c) return NULL;
    return c->description;
}

// Setter
void cmd_set_description(cmd* c, const char* description) {
    if (c) {
        c->description = description;
    }
}