#include "node.h"

TreeNode *createNode(char *type, ChildNodes *childNodes, char *value) {
    TreeNode *node = malloc(sizeof(TreeNode));
    if (childNodes) {
        node->childrenNumber = childNodes->size;
        node->childNodes = childNodes->childNodes;
    } else {
        node->childrenNumber = 0;
        node->childNodes = NULL;
    }
    node->type = type;
    char *buf = malloc(1024 * sizeof(char));
    strcpy(buf, value);
    node->value = buf;
    allNodes[allNodesCount] = node;
    allNodesCount++;
    return node;
}

void printNodeVal(TreeNode *node, FILE *output_file) {
    fprintf(output_file,"node%d", node->id);
    fprintf(output_file,"([");
    fprintf(output_file,"Type: %s", node->type);
    if (strlen(node->value) > 0) {
        fprintf(output_file,", Value: %s", node->value);
    }
    fprintf(output_file,"])");
}

void printNodeRelation(TreeNode *node, FILE *output_file) {
    for (int i = 0; i < node->childrenNumber; ++i) {
        printNodeVal(node, output_file);
        TreeNode *childNode = node->childNodes[i];
        fprintf(output_file," --> ");
        printNodeVal(childNode, output_file);
        fprintf(output_file,"\n");

        printNodeRelation(childNode, output_file);
    }
}

void printTree(TreeNode **allNodes, int allNodesCount, FILE *output_file) {
    for (int i = 0; i < allNodesCount; ++i) {
        allNodes[i]->id = i;
    }
    fprintf(output_file, "flowchart TB\n");
    printNodeRelation(allNodes[allNodesCount - 1], output_file);

    printNodeVal(allNodes[allNodesCount - 1], output_file);
    fprintf(output_file,"\n");
}

ChildNodes *mallocChildNodes(int size, TreeNode **nodesArg) {
    if (size > 0 && nodesArg) {
        TreeNode **nodes = malloc(sizeof(TreeNode *) * size);
        int notNullCounter = 0;
        for (int i = 0; i < size; ++i) {
            if (nodesArg[i]) {
                nodes[notNullCounter] = nodesArg[i];
                notNullCounter++;
            }
        }
        ChildNodes *childNodes = malloc(sizeof(ChildNodes));
        childNodes->childNodes = nodes;
        childNodes->size = notNullCounter;
        return childNodes;
    } else {
        return NULL;
    }
}

TreeNode *initBinaryListNode(TreeNode *valueNode, TreeNode *previousListNode, bool hasNext, char *nodeName) {
    int childNumber = 1;
    if (hasNext) {
        childNumber = 2;
    }
    TreeNode *listNode = mallocTreeNode(nodeName, NULL, childNumber);
    listNode->childNodes[0] = valueNode;
    previousListNode->childNodes[1] = listNode;
    return listNode;
}

TreeNode *initListStatementNode(TreeNode *statementNode, TreeNode *previousStatementListNode, bool hasNext) {
    return initBinaryListNode(
            statementNode,
            previousStatementListNode,
            hasNext,
            "listStatement"
    );
}

TreeNode *initListExprNode(TreeNode *exprNode, TreeNode *previousExprListNode, bool hasNext) {
    return initBinaryListNode(
            exprNode,
            previousExprListNode,
            hasNext,
            "listExpr"
    );
}