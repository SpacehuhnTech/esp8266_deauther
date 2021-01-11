/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef parser_h
#define parser_h

#include "parser_types.h"

// ===== Word Node ===== //
word_node* word_node_create(const char* str, size_t len);
word_node* word_node_destroy(word_node* n);
word_node* word_node_destroy_rec(word_node* n);

// ===== Word List ===== //
word_list* word_list_create();
word_list* word_list_destroy(word_list* l);

void word_list_push(word_list* l, word_node* n);
word_node* word_list_get(word_list* l, size_t i);

// ===== Line Node ==== //
line_node* line_node_create(const char* str, size_t len);
word_node* line_node_destroy(line_node* n);
word_node* line_node_destroy_rec(line_node* n);

// ===== Line List ===== //
line_list* line_list_create();
line_list* line_list_destroy(line_list* l);

void line_list_push(line_list* l, line_node* n);
line_node* line_list_get(line_list* l, size_t i);

// ===== Parser ===== //
word_list* parse_words(const char* str, size_t len);
line_list* parse_lines(const char* str, size_t len);

#endif /* ifndef parser_h */