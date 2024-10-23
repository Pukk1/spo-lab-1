//
// Created by Ivan on 20.10.2024.
//

#include "execution.h"

typedef struct Array Array;

const int START_ARRAY_SIZE = 8;

struct Array {
  int size;
  int nextPosition;
  void **elements;
};

Array exceptions;

// добавление в массив с возможным динамическим расширением
void addToList(Array *currentArray, void *element) {
  TreeNode **nodes;
  if (currentArray->size != currentArray->nextPosition) {
    nodes = currentArray->elements;
  } else {
    nodes = malloc(sizeof(TreeNode *) * 2 * currentArray->size);
    for (int i = 0; i < currentArray->size; ++i) {
      nodes[i] = currentArray->elements[i];
    }
    free(currentArray->elements);
  }
  nodes[currentArray->nextPosition] = element;
  currentArray->nextPosition += 1;
}

// утилита для получения всех node дерева разбора в виде массива (вызвано
// бинарной реализацией листов)
Array findListItemsUtil(TreeNode *treeNode) {
  TreeNode **nodes = malloc(sizeof(TreeNode *) * START_ARRAY_SIZE);
  Array items = {START_ARRAY_SIZE, 0, nodes};

  TreeNode *currentListNode = treeNode;
  do {
    addToList(&items, currentListNode->childNodes[0]);
    if (currentListNode->childrenNumber == 0) {
      currentListNode = NULL;
    } else if (currentListNode->childrenNumber == 2) {
      currentListNode = currentListNode->childNodes[1];
    } else {
      char exceptionText[1024];
      sprintf(exceptionText,
              "Exception in list parsing more than two by element id %d",
              currentListNode->id);
      char *exception = malloc(sizeof(exceptionText));
      addToList(&exceptions, exception);
      return items;
    }
  } while (currentListNode != NULL);
  return items;
}

// получение корневого элемента из результатов парсинга
TreeNode *findSourceNode(FilenameParseTree *input) {
  TreeNode **inputNodes = input->tree->nodes;
  int inputNodesSize = input->tree->size;
  TreeNode *sourceNode = inputNodes[inputNodesSize - 1];
  return sourceNode;
}

// получение списка функций из корневого элемента
Array findSourceItems(TreeNode *source) {
  TreeNode *sourceItemsList = source->childNodes[0];
  return findListItemsUtil(sourceItemsList);
}

// созадние базовго блока
ExecutionNode *executionBlock(TreeNode *node, ExecutionNode *baseNode) {
  //  TODO
}

FunExecution *executionGraph(FilenameParseTree *input) {
  TreeNode *sourceNode = findSourceNode(input);
  Array sourceItems = findSourceItems(sourceNode);
}
