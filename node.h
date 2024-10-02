#ifndef SPO_LAB1_NODE_H
#define SPO_LAB1_NODE_H

#include "parser.h"

typedef struct TreeNode TreeNode;
typedef struct ChildNodes ChildNodes;

extern TreeNode **allNodes;
extern int allNodesCount;

struct ChildNodes {
    int size;
    TreeNode **childNodes;
};

struct TreeNode {
    char *type;
    TreeNode **childNodes;
    int childrenNumber;
    char *value;
    int id;
};

void printTree(TreeNode **allNodes, int allNodesCount, FILE *output_file);

TreeNode *createNode(char *type, ChildNodes *childNodes, char *value);

ChildNodes *mallocChildNodes(int size, TreeNode **nodesArg);

#endif //SPO_LAB1_NODE_H
