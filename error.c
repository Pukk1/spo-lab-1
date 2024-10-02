#include <stdio.h>
#include "error.h"
#include "parser.h"
#include "string.h"
#include "parser.tab.h"

void yyerror(const char *s) {
//    yylloc;

    char error[1024] = "";
    sprintf(error, "Error | Line: %d: %s\n", yylineno, s);
    errors[errorsCount] = malloc(strlen(error));
    sprintf(errors[errorsCount], "%s", error);
    errorsCount += 1;
}