//
// Created by Ivan on 14.06.2025.
//

#include "exception_util.h"

List *exceptions;

void addException(char *text) {
    char *exception = mallocString(text);
    addToList(exceptions, exception);
}