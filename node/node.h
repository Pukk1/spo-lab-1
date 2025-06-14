#ifndef SPO_LAB1_NODE_H
#define SPO_LAB1_NODE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "node_types.h"
#include "../parse/parser.h"

void printTree(TreeNode **allNodes, int allNodesCount, FILE *output_file);

TreeNode *createNode(char *type, ChildNodes *childNodes, char *value);

ChildNodes *mallocChildNodes(int size, TreeNode **nodesArg);

#endif //SPO_LAB1_NODE_H
