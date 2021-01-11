/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

 #include "arg.h"

#include <stdlib.h>     // malloc()
#include <string.h>     // memcpy()

#include "comparator.h" // compare

// ===== Arg ===== //

// Constructors
arg* arg_create(const char* name, const char* default_val, unsigned int mode, unsigned int req) {
    arg* a = (arg*)malloc(sizeof(arg));

    a->name        = name;
    a->default_val = default_val;
    a->val         = NULL;
    a->mode        = mode % 3;
    a->req         = req % 2;
    a->set         = ARG_UNSET;
    a->next        = NULL;

    return a;
}

arg* arg_create_opt(const char* name, const char* default_val) {
    return arg_create(name, default_val, ARG_DEFAULT, ARG_OPT);
}

arg* arg_create_req(const char* name) {
    return arg_create(name, NULL, ARG_DEFAULT, ARG_REQ);
}

arg* arg_create_opt_positional(const char* name, const char* default_value) {
    return arg_create(name, default_value, ARG_POS, ARG_OPT);
}

arg* arg_create_req_positional(const char* name) {
    return arg_create(name, NULL, ARG_POS, ARG_REQ);
}

arg* arg_create_flag(const char* name, const char* default_value) {
    return arg_create(name, default_value, ARG_FLAG, ARG_OPT);
}

// Copy & Move Constructors

arg* arg_copy(arg* a) {
    if (!a) return NULL;

    arg* na = (arg*)malloc(sizeof(arg));

    na->name        = a->name;
    na->default_val = a->default_val;
    na->val         = NULL;
    na->mode        = a->mode;
    na->req         = a->req;
    na->set         = a->set;
    na->next        = NULL;

    if (a->val) {
        na->val = (char*)malloc(strlen(a->val) + 1);
        strcpy(na->val, a->val);
        na->set = ARG_SET;
    }

    return na;
}

arg* arg_copy_rec(arg* a) {
    if (!a) return NULL;

    arg* na = arg_copy(a);

    na->next = arg_copy_rec(a->next);

    return na;
}

arg* arg_move(arg* a) {
    if (!a) return NULL;

    arg* na = (arg*)malloc(sizeof(arg));

    na->name        = a->name;
    na->default_val = a->default_val;
    na->val         = a->val;
    na->mode        = a->mode;
    na->req         = a->req;
    na->set         = a->set;
    na->next        = NULL;

    a->val = NULL;
    a->set = ARG_UNSET;

    return na;
}

arg* arg_move_rec(arg* a) {
    if (!a) return NULL;

    arg* na = arg_move(a);

    na->next = arg_move_rec(a->next);

    return na;
}

// Destructors
arg* arg_destroy(arg* a) {
    if (a) {
        arg_reset(a);
        free(a);
    }
    return NULL;
}

arg* arg_destroy_rec(arg* a) {
    if (a) {
        arg_destroy_rec(a->next);
        arg_destroy(a);
    }
    return NULL;
}

// Reset
void arg_reset(arg* a) {
    if (a) {
        if (a->val) {
            free(a->val);
            a->val = NULL;
        }
        a->set = ARG_UNSET;
    }
}

void arg_reset_rec(arg* a) {
    if (a) {
        arg_reset(a);
        arg_reset_rec(a->next);
    }
}

// Comparisons
int arg_name_equals(arg* a, const char* name, size_t name_len, int case_sensetive) {
    if (!a) return ARG_NAME_UNEQUALS;
    return compare(name, name_len, a->name, case_sensetive) == COMPARE_EQUAL ? ARG_NAME_EQUALS : ARG_NAME_UNEQUALS;
}

int arg_equals(arg* a, arg* b, int case_sensetive) {
    if (a == b) return ARG_NAME_EQUALS;
    if (!a || !b) return ARG_NAME_UNEQUALS;
    return arg_name_equals(a, b->name, strlen(b->name), case_sensetive);
}

// Getter
const char* arg_get_value(arg* a) {
    if (a) {
        if (a->val) return a->val;
        if (a->default_val) return a->default_val;
    }
    return "";
}

// Setter
int arg_set_value(arg* a, const char* val, size_t val_size) {
    if (a) {
        if (val && (val_size > 0)) {
            if (a->set) arg_reset(a);
            a->val = (char*)malloc(val_size + 1);

            size_t i = 0;
            size_t j = 0;

            int escaped  = 0;
            int in_quote = 0;

            while (i < val_size) {
                if ((val[i] == '\\') && (escaped == 0)) {
                    escaped = 1;
                } else if ((val[i] == '"') && (escaped == 0)) {
                    in_quote = !in_quote;
                } else {
                    a->val[j++] = val[i];
                    escaped     = 0;
                }

                ++i;
            }

            if (in_quote) {
                free(a->val);
                a->val = NULL;
                return ARG_VALUE_FAIL;
            }

            while (j <= val_size) {
                a->val[j++] = '\0';
            }
        }

        a->set = ARG_SET;
        return ARG_VALUE_SUCCESS;
    }

    return ARG_VALUE_FAIL;
}