//
// Created by Ivan on 20.10.2024.
//

#ifndef SPO_LAB1_EXECUTION_H
#define SPO_LAB1_EXECUTION_H

#include "parser.h"

typedef struct FilenameParseTree FilenameParseTree;
typedef struct ExecutionNode ExecutionNode;
typedef struct FunExecution FunExecution;
typedef struct Array Array;

struct ExecutionNode {
  char *text;
  ExecutionNode *definitely;    // безусловный переход
  ExecutionNode *conditionally; // условный переход
  TreeNode *operationTree;
  int id;
};

struct Array {
    int size;
    int nextPosition;
    void **elements;
};

struct FunExecution {
  char *name;
  char *filename;
  TreeNode *signature;
  TreeNode *funCalls;
  ExecutionNode *nodes;
  char **errors;
  int errorsCount;
};

struct FilenameParseTree {
  char *filename;
  ParseResult *tree;
};

Array *executionGraph(FilenameParseTree *input, int size);

#endif // SPO_LAB1_EXECUTION_H
