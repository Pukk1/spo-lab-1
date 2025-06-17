#ifndef SPO_LAB1_NODE_H
#define SPO_LAB1_NODE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "node_types.h"
#include "../parse/parser.h"
#include <stdbool.h>
#include "../common/node/tree_node_util.h"

void printTree(TreeNode **allNodes, int allNodesCount, FILE *output_file);

TreeNode *createNode(char *type, ChildNodes *childNodes, char *value);

ChildNodes *mallocChildNodes(int size, TreeNode **nodesArg);

TreeNode *initListExprNode(TreeNode *exprNode, TreeNode *previousExprListNode, bool hasNext);

TreeNode *initListStatementNode(TreeNode *statementNode, TreeNode *previousStatementListNode, bool hasNext);

#endif //SPO_LAB1_NODE_H
