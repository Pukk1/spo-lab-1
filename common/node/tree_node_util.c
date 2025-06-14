//
// Created by Ivan on 14.06.2025.
//

#include "tree_node_util.h"

TreeNode *mallocTreeNode(char *type, char *value, int nodeNumber) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->id = getNextExecutionId();
    if (type) {
        node->type = mallocString(type);
    } else {
        node->type = NULL;
    }
    if (value) {
        node->value = mallocString(value);
    } else {
        node->value = NULL;
    }
    node->childNodes = malloc(sizeof(TreeNode *) * nodeNumber);
    node->childrenNumber = nodeNumber;
    return node;
}

// утилита для получения всех node дерева разбора в виде массива (вызвано
// бинарной реализацией листов)
List findListItemsUtil(TreeNode *treeNode) {
    TreeNode **nodes = malloc(sizeof(TreeNode *) * START_ARRAY_SIZE);
    List items = {nodes, 0, START_ARRAY_SIZE};

    TreeNode *currentListNode = treeNode;
    do {
        if (currentListNode->childrenNumber == 0) {
            currentListNode = NULL;
        } else if (currentListNode->childrenNumber == 1) {
            addToList(&items, currentListNode->childNodes[0]);
            currentListNode = NULL;
        } else if (currentListNode->childrenNumber == 2) {
            addToList(&items, currentListNode->childNodes[0]);
            currentListNode = currentListNode->childNodes[1];
        } else {
            char exceptionText[1024];
            sprintf(exceptionText,
                    "Exception in list parsing more than two by element id %d",
                    currentListNode->id);
//            addException(exceptionText);
            return items;
        }
    } while (currentListNode != NULL);
    return items;
}