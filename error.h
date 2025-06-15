#ifndef SPO_LAB1_ERROR_H
#define SPO_LAB1_ERROR_H

#include <malloc.h>
#include "string.h"
#include <stdio.h>
#include "parser.tab.h"

extern int yylineno;

void yyerror(const char *s);

#endif //SPO_LAB1_ERROR_H
