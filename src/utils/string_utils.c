#include "utils/string_utils.h"
#include <stdio.h>
#include <stdlib.h>

char *duplicate_string(const char *str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char *new_str = (char *)malloc(len);
    if (new_str == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }
    strcpy_s (new_str, len, str);
    return new_str;
}
