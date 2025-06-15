#include "error.h"
#include "common/exception/exception_util.h"

void yyerror(const char *s) {
//    yylloc;

    char error[1024] = "";
    sprintf(error, "Error | Line: %d: %s\n", yylineno, s);
    printException(error);
}