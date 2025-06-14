#ifndef SPO_LAB1_ERROR_H
#define SPO_LAB1_ERROR_H

#include <malloc.h>
#include "string.h"
#include <stdio.h>
#include "parser.tab.h"

extern int yylineno;
extern char **errors;
extern int errorsCount;

void yyerror(const char *s);

#endif //SPO_LAB1_ERROR_H
