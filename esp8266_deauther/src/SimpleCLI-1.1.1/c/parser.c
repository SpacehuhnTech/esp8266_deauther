/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include "parser.h"

#include <stdlib.h> // malloc

// ===== Word Node ===== //
word_node* word_node_create(const char* str, size_t len) {
    word_node* n = (word_node*)malloc(sizeof(word_node));

    n->str  = str;
    n->len  = len;
    n->next = NULL;
    return n;
}

word_node* word_node_destroy(word_node* n) {
    if (n) {
        free(n);
    }
    return NULL;
}

word_node* word_node_destroy_rec(word_node* n) {
    if (n) {
        word_node_destroy_rec(n->next);
        word_node_destroy(n);
    }
    return NULL;
}

// ===== Word List ===== //
word_list* word_list_create() {
    word_list* l = (word_list*)malloc(sizeof(word_list));

    l->first = NULL;
    l->last  = NULL;
    l->size  = 0;
    return l;
}

word_list* word_list_destroy(word_list* l) {
    if (l) {
        word_node_destroy_rec(l->first);
        free(l);
    }
    return NULL;
}

void word_list_push(word_list* l, word_node* n) {
    if (l && n) {
        if (l->last) {
            l->last->next = n;
        } else {
            l->first = n;
        }

        l->last = n;
        ++(l->size);
    }
}

word_node* word_list_get(word_list* l, size_t i) {
    if (!l) return NULL;

    size_t j;
    word_node* h = l->first;

    for (j = 0; j < i && h; ++j) {
        h = h->next;
    }

    return h;
}

// ===== Line Node ==== //
line_node* line_node_create(const char* str, size_t len) {
    line_node* n = (line_node*)malloc(sizeof(line_node));

    n->str   = str;
    n->len   = len;
    n->words = NULL;
    n->next  = NULL;

    return n;
}

word_node* line_node_destroy(line_node* n) {
    if (n) {
        word_list_destroy(n->words);
        free(n);
    }
    return NULL;
}

word_node* line_node_destroy_rec(line_node* n) {
    if (n) {
        line_node_destroy_rec(n->next);
        line_node_destroy(n);
    }
    return NULL;
}

// ===== Line List ===== //
line_list* line_list_create() {
    line_list* l = (line_list*)malloc(sizeof(line_list));

    l->first = NULL;
    l->last  = NULL;
    l->size  = 0;

    return l;
}

line_list* line_list_destroy(line_list* l) {
    if (l) {
        line_node_destroy_rec(l->first);
        free(l);
    }
    return NULL;
}

void line_list_push(line_list* l, line_node* n) {
    if (l && n) {
        if (l->last) {
            l->last->next = n;
        } else {
            l->first = n;
        }

        l->last = n;

        ++(l->size);
    }
}

line_node* line_list_get(line_list* l, size_t i) {
    if (!l) return NULL;

    size_t j;
    line_node* h = l->first;

    for (j = 0; j < i && h; ++j) {
        h = h->next;
    }

    return h;
}

// ===== Parser ===== //
word_list* parse_words(const char* str, size_t len) {
    word_list* l = word_list_create();

    if (len == 0) return l;

    // Go through string and look for space to split it into words
    word_node* n = NULL;

    size_t i = 0; // current index
    size_t j = 0; // start index of word

    int escaped      = 0;
    int ignore_space = 0;

    for (i = 0; i <= len; ++i) {
        if ((str[i] == '\\') && (escaped == 0)) {
            escaped = 1;
        } else if ((str[i] == '"') && (escaped == 0)) {
            ignore_space = !ignore_space;
        } else if ((i == len) || ((str[i] == ' ') && (ignore_space == 0) && (escaped == 0))) {
            size_t k = i - j; // length of word

            // for every word, add to list
            if (k > 0) {
                n = word_node_create(&str[j], k);
                word_list_push(l, n);
            }

            j = i + 1; // reset start index of word
        } else if (escaped == 1) {
            escaped = 0;
        }
    }

    return l;
}

line_list* parse_lines(const char* str, size_t len) {
    line_list* l = line_list_create();

    if (len == 0) return l;

    // Go through string and look for /r and /n to split it into lines
    line_node* n = NULL;

    size_t i = 0; // current index
    size_t j = 0; // start index of line

    int ignore_delimiter = 0;
    int delimiter        = 0;
    int linebreak        = 0;
    int endofline        = 0;

    for (i = 0; i <= len; ++i) {
        if ((str[i] == '"') && ((str[i-1] != '\\') || (i==0))) ignore_delimiter = !ignore_delimiter;

        delimiter = (str[i] == ';' && str[i+1] == ';' && !ignore_delimiter && (i == 0 || str[i-1] != '\\'));
        linebreak = ((str[i] == '\r') || (str[i] == '\n')) && !ignore_delimiter;
        endofline = (i == len);

        if (linebreak || endofline || delimiter) {
            size_t k = i - j; // length of line

            // for every line, parse_words and add to list
            if (k > 0) {
                n        = line_node_create(&str[j], k);
                n->words = parse_words(&str[j], k);
                line_list_push(l, n);
            }

            if (delimiter) ++i;

            j = i+1; // reset start index of line
        }
    }

    return l;
}