//
// Created by Ivan on 14.06.2025.
//

#include "file_work.h"

void fprintlnWithArg(char *message, char *arg, FILE *file) {
    fprintf(file, "%s %s\n", message, arg);
}

void fprintln(char *message, FILE *file) {
    fprintf(file, "%s\n", message);
}
