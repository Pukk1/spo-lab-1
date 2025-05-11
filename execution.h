//
// Created by Ivan on 20.10.2024.
//

#ifndef SPO_LAB1_EXECUTION_H
#define SPO_LAB1_EXECUTION_H

#include "parser.h"
#include "util.h"

typedef struct FilenameParseTree FilenameParseTree;
typedef struct ExecutionNode ExecutionNode;
typedef struct SourceItemExecution SourceItemExecution;
typedef struct ListingNode ListingNode;

struct ExecutionNode {
    char *text;
    ExecutionNode *definitely;    // безусловный переход
    ExecutionNode *conditionally; // условный переход
    TreeNode *operationTree;
    int id;
    int printed;
    ListingNode *listingNode;
};

struct SourceItemExecution {
    char *name;
    char *filename;
    TreeNode *funCalls;
    ExecutionNode *nodes;
    char **errors;
    int errorsCount;
    bool isMethod;
};

struct FilenameParseTree {
    char *filename;
    ParseResult *tree;
};

List *executionGraph(FilenameParseTree *input, int size);

void printExecution(SourceItemExecution *funExecution, FILE *outputFunCallFile, FILE *outputOperationTreesFile,
                    FILE *outputExecutionFile);

#endif // SPO_LAB1_EXECUTION_H
