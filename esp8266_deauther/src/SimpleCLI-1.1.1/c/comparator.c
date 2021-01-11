/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#include <string.h> // strlen

#include "comparator.h"

// My own implementation, because the default one in ctype.h make problems on older ESP8266 SDKs
char to_lower(char c) {
    if ((c >= 65) && (c <= 90)) {
        return (char)(c + 32);
    }
    return c;
}

int compare(const char* user_str, size_t user_str_len, const char* templ_str, int case_sensetive) {
    if (user_str == templ_str) return COMPARE_EQUAL;

    // null check string pointers
    if (!user_str || !templ_str) return COMPARE_UNEQUAL;

    // string lengths
    size_t str_len = user_str_len; // strlen(user_str);
    size_t key_len = strlen(templ_str);

    // when same length, it there is no need to check for slashes or commas
    if (str_len == key_len) {
        for (size_t i = 0; i < key_len; i++) {
            if (case_sensetive == COMPARE_CASE_SENSETIVE) {
                if (user_str[i] != templ_str[i]) return COMPARE_UNEQUAL;
            } else {
                if (to_lower(user_str[i]) != to_lower(templ_str[i])) return COMPARE_UNEQUAL;
            }
        }
        return COMPARE_EQUAL;
    }

    // string can't be longer than templ_str (but can be smaller because of  '/' and ',')
    if (str_len > key_len) return COMPARE_UNEQUAL;

    unsigned int res_i = 0;
    unsigned int a     = 0;
    unsigned int b     = 0;
    unsigned int res   = 1;

    while (a < str_len && b < key_len) {
        if (templ_str[b] == '/') {
            // skip slash in templ_str
            ++b;
        } else if (templ_str[b] == ',') {
            // on comma increment res_i and reset str-index
            ++b;
            a = 0;
            ++res_i;
        }

        // compare character
        if (case_sensetive == COMPARE_CASE_SENSETIVE) {
            if (user_str[a] != templ_str[b]) res = 0;
        } else {
            if (to_lower(user_str[a]) != to_lower(templ_str[b])) res = 0;
        }

        // comparison incorrect or string checked until the end and templ_str not checked until the end
        if (!res || ((a == str_len - 1) &&
                     (templ_str[b + 1] != ',') &&
                     (templ_str[b + 1] != '/') &&
                     (templ_str[b + 1] != '\0'))) {
            // fast forward to next comma
            while (b < key_len && templ_str[b] != ',') b++;
            res = 1;
        } else {
            // otherwise icrement indices
            ++a;
            ++b;
        }
    }

    // comparison correct AND string checked until the end AND templ_str checked until the end
    if (res && (a == str_len) &&
        ((templ_str[b] == ',') ||
         (templ_str[b] == '/') ||
         (templ_str[b] == '\0'))) return COMPARE_EQUAL;  // res_i

    return COMPARE_UNEQUAL;
}