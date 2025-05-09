//
// Created by Ivan on 09.05.2025.
//

#ifndef SPO_LAB1_UTIL_H
#define SPO_LAB1_UTIL_H

#include <string.h>
#include <stdbool.h>
#include <malloc.h>
#include "node.h"

typedef struct List List;

struct List {
    void **elements;
    int size;
    int capacity;
};

void addToList(List *currentList, void *element);
List findListItemsUtil(TreeNode *treeNode);
List *initEmptyList();

#endif //SPO_LAB1_UTIL_H
