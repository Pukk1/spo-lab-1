//
// Created by Ivan on 14.06.2025.
//

#include "string_util.h"

char *mallocString(char *text) {
    char *pointer = malloc(sizeof(char) * 1024);
    sprintf(pointer, "%s", text);
    return pointer;
}
