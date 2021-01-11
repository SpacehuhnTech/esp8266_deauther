/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef parser_types_h
#define parser_types_h

#include <stddef.h> // size_t

typedef struct word_node {
    const char      * str;
    size_t            len;
    struct word_node* next;
} word_node;

typedef struct word_list {
    struct word_node* first;
    struct word_node* last;
    size_t            size;
} word_list;

typedef struct line_node {
    const char      * str;
    size_t            len;
    struct word_list* words;
    struct line_node* next;
} line_node;

typedef struct line_list {
    struct line_node* first;
    struct line_node* last;
    size_t            size;
} line_list;

#endif /* ifndef parser_types_h */