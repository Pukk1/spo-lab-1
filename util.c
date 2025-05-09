//
// Created by Ivan on 09.05.2025.
//

#include "util.h"

const int START_ARRAY_SIZE = 8;

// добавление в массив с возможным динамическим расширением
void addToList(List *currentArray, void *element) {
    void **nodes;
    if (currentArray->capacity != currentArray->size) {
        nodes = currentArray->elements;
    } else {
        nodes = malloc(sizeof(void *) * 2 * currentArray->size);
        for (int i = 0; i < currentArray->size; ++i) {
            nodes[i] = currentArray->elements[i];
        }
        free(currentArray->elements);
        currentArray->elements = nodes;
    }
    nodes[currentArray->size] = element;
    currentArray->size += 1;
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

List *initEmptyList() {
    void** nodes = malloc(sizeof(void *) * START_ARRAY_SIZE);
    List *list = malloc(sizeof(List));
    list->size = 0;
    list->capacity = START_ARRAY_SIZE;
    list->elements = nodes;
    return list;
}
