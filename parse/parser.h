#ifndef SPO_LAB1_PARSER_H
#define SPO_LAB1_PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include "parse_types.h"
#include "../error.h"

extern TreeNode **allNodes;
extern int allNodesCount;
extern int yyparse();
extern FILE *yyin;

ParseResult *parse(FILE *file);

void freeMem(ParseResult *parseResult);

#endif //SPO_LAB1_PARSER_H
