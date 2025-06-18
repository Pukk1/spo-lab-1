#include "parser.h"

TreeNode **allNodes;
int allNodesCount;


ParseResult *parse(FILE *file) {
    allNodes = malloc(5000 * sizeof(TreeNode *));
    allNodesCount = 0;

    yyin = file;
    yyparse();

    ParseResult *parseResult = malloc(sizeof(ParseResult));
    parseResult->nodes = allNodes;
    parseResult->size = allNodesCount;

    return parseResult;
}

void freeMem(ParseResult *parseResult) {
    for (int i = parseResult->size - 1; i >= 0; --i) {
        TreeNode *node = parseResult->nodes[i];

        free(node->childNodes);
        free(node->value);
        free(node);
    }
    free(parseResult);
}