#include "parser.h"

TreeNode **allNodes;
int allNodesCount;
char **errors;
int errorsCount;


ParseResult *parse(FILE *file) {
    allNodes = malloc(1024 * sizeof(TreeNode *));
    allNodesCount = 0;
    errors = malloc(1024 * sizeof(char *));
    errorsCount = 0;

    yyin = file;
    yyparse();

    ParseResult *parseResult = malloc(sizeof(ParseResult));
    parseResult->nodes = allNodes;
    parseResult->size = allNodesCount;
    parseResult->errors = errors;
    parseResult->errorsCount = errorsCount;

    return parseResult;
}

void freeMem(ParseResult *parseResult) {
    for (int i = parseResult->size - 1; i >= 0; --i) {
        TreeNode *node = parseResult->nodes[i];

        free(node->childNodes);
        free(node->value);
        free(node);
    }
    for (int i = 0; i <parseResult->errorsCount; ++i) {
        free(parseResult->errors[i]);
    }
    free(parseResult->errors);
    free(parseResult);
}