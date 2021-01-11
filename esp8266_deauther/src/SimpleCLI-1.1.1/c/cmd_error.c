/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include "cmd_error.h"
#include <stdlib.h> // malloc, memcpy
#include <string.h> // strlen, strcpy

// ===== CMD Parse Error ===== //

// Constructors
cmd_error* cmd_error_create(int mode, cmd* command, arg* argument, word_node* data) {
    cmd_error* e = (cmd_error*)malloc(sizeof(cmd_error));

    e->mode     = mode;
    e->command  = command;
    e->argument = argument;
    e->data     = NULL;
    e->next     = NULL;

    if (data && (data->len > 0)) {
        e->data = (char*)malloc(data->len + 1);
        memcpy(e->data, data->str, data->len);
        e->data[data->len] = '\0';
    }

    return e;
}

cmd_error* cmd_error_create_null_ptr(cmd* c) {
    return cmd_error_create(CMD_NULL_PTR, c, NULL, NULL);
};

cmd_error* cmd_error_create_empty_line(cmd* c) {
    return cmd_error_create(CMD_EMPTY_LINE, c, NULL, NULL);
}

cmd_error* cmd_error_create_parse_success(cmd* c) {
    return cmd_error_create(CMD_PARSE_SUCCESS, c, NULL, NULL);
}

cmd_error* cmd_error_create_not_found(cmd* c, word_node* cmd_name) {
    return cmd_error_create(CMD_NOT_FOUND, c, NULL, cmd_name);
}

cmd_error* cmd_error_create_unknown_arg(cmd* c, word_node* arg_name) {
    return cmd_error_create(CMD_UNKOWN_ARG, c, NULL, arg_name);
}

cmd_error* cmd_error_create_missing_arg(cmd* c, arg* a) {
    return cmd_error_create(CMD_MISSING_ARG, c, a, NULL);
}

cmd_error* cmd_error_create_unclosed_quote(cmd* c, arg* a, word_node* arg_value) {
    return cmd_error_create(CMD_UNCLOSED_QUOTE, c, a, arg_value);
}

// Copy Constructors
cmd_error* cmd_error_copy(cmd_error* e) {
    if (!e) return NULL;

    cmd_error* ne = (cmd_error*)malloc(sizeof(cmd_error));

    ne->mode     = e->mode;
    ne->command  = e->command;
    ne->argument = e->argument;
    ne->data     = e->data;
    ne->next     = e->next;

    if (ne->data) {
        ne->data = (char*)malloc(strlen(e->data) + 1);
        strcpy(ne->data, e->data);
    }

    return ne;
}

cmd_error* cmd_error_copy_rec(cmd_error* e) {
    if (!e) return NULL;

    cmd_error* ne = cmd_error_copy(e);

    ne->next = cmd_error_copy_rec(e->next);

    return ne;
}

// Destructors
cmd_error* cmd_error_destroy(cmd_error* e) {
    if (e) {
        if (e->data) free(e->data);
        free(e);
    }
    return NULL;
}

cmd_error* cmd_error_destroy_rec(cmd_error* e) {
    if (e) {
        cmd_error_destroy_rec(e->next);
        cmd_error_destroy(e);
    }
    return NULL;
}

// Push
cmd_error* cmd_error_push(cmd_error* l, cmd_error* e, int max_size) {
    if (max_size < 1) {
        cmd_error_destroy_rec(l);
        cmd_error_destroy(e);
        return NULL;
    }

    if (!l) return e;

    cmd_error* h = l;
    int i        = 1;

    while (h->next) {
        h = h->next;
        ++i;
    }

    h->next = e;

    // Remove first element if list is too big
    if (i > max_size) {
        cmd_error* ptr = l;
        l = l->next;
        cmd_error_destroy(ptr);
    }

    return l;
}