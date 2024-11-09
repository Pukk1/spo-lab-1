//
// Created by Ivan on 20.10.2024.
//

#include "execution.h"
#include <string.h>

typedef struct Array Array;

const int START_ARRAY_SIZE = 8;

struct Array {
  int size;
  int nextPosition;
  void **elements;
};

Array exceptions;
int currentExecutionId = -1;

ExecutionNode *executionBlock(TreeNode *treeNode, ExecutionNode *baseNode,
                              ExecutionNode *nextNode,
                              ExecutionNode *breakNode);

char *mallocString(char *text) {
  char *pointer = malloc(sizeof(char) * 1024);
  sprintf(pointer, "%s", text);
  return pointer;
}

// добавление в массив с возможным динамическим расширением
void addToList(Array *currentArray, void *element) {
  void **nodes;
  if (currentArray->size != currentArray->nextPosition) {
    nodes = currentArray->elements;
  } else {
    nodes = malloc(sizeof(void *) * 2 * currentArray->size);
    for (int i = 0; i < currentArray->size; ++i) {
      nodes[i] = currentArray->elements[i];
    }
    free(currentArray->elements);
  }
  nodes[currentArray->nextPosition] = element;
  currentArray->nextPosition += 1;
}

void addException(char *text) {
  char *exception = mallocString(text);
  addToList(&exceptions, exception);
}

int getNextExecutionId() {
  currentExecutionId++;
  return currentExecutionId;
}

// утилита для получения всех node дерева разбора в виде массива (вызвано
// бинарной реализацией листов)
Array findListItemsUtil(TreeNode *treeNode) {
  TreeNode **nodes = malloc(sizeof(TreeNode *) * START_ARRAY_SIZE);
  Array items = {START_ARRAY_SIZE, 0, nodes};

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
      addException(exceptionText);
      return items;
    }
  } while (currentListNode != NULL);
  return items;
}

// получение корневого элемента из результатов парсинга
TreeNode *findSourceNode(FilenameParseTree input) {
  TreeNode **inputNodes = input.tree->nodes;
  int inputNodesSize = input.tree->size;
  TreeNode *sourceNode = inputNodes[inputNodesSize - 1];
  return sourceNode;
}

// получение списка функций из корневого элемента
Array findSourceItems(TreeNode *source) {
  if (source->childrenNumber != 0) {
    TreeNode *sourceItemsList = source->childNodes[0];
    return findListItemsUtil(sourceItemsList);
  } else {
    return (Array){0, 0, NULL};
  }
}

ExecutionNode *initExecutionNode(char *text) {
  ExecutionNode *node = malloc(sizeof(ExecutionNode));
  node->id = getNextExecutionId();
  node->text = mallocString(text);
  node->definitely = NULL;
  node->conditionally = NULL;
  return node;
}

// создание блока listStatement
ExecutionNode *executionListStatementBlock(TreeNode *treeNode,
                                           ExecutionNode *baseNode,
                                           ExecutionNode *nextNode,
                                           ExecutionNode *breakNode) {
  ExecutionNode *tmpNextNode = nextNode;
  if (treeNode->childrenNumber == 2) {
    tmpNextNode =
        executionBlock(treeNode->childNodes[1], NULL, nextNode, breakNode);
  }
  ExecutionNode *block = initExecutionNode("");
  block->definitely =
      executionBlock(treeNode->childNodes[0], NULL, tmpNextNode, breakNode);
  return block;
}

// созадние блока
ExecutionNode *executionBlock(TreeNode *treeNode, ExecutionNode *baseNode,
                              ExecutionNode *nextNode,
                              ExecutionNode *breakNode) {
  if (strcmp(treeNode[0].type, "listStatement")) {
    return executionListStatementBlock(treeNode, baseNode, nextNode, breakNode);
  } else {
    char exceptionText[1024];
    sprintf(exceptionText,
            "Exception in tree node parsing id --> %d illegal type --> %s",
            treeNode[0].id, treeNode[0].type);
    addException(exceptionText);
    ExecutionNode *exceptionNode = initExecutionNode(exceptionText);
    exceptionNode->definitely = nextNode;
    return exceptionNode;
  }
}

ExecutionNode *initGraph(TreeNode *sourceItem) {
  ExecutionNode *startNode = initExecutionNode("START");
  ExecutionNode *endNode = initExecutionNode("FINISH");
  TreeNode *funcDef = sourceItem->childNodes[0];
  if (funcDef->childrenNumber == 2) {
    ExecutionNode *listStatementNode =
        executionBlock(funcDef->childNodes[1], NULL, endNode, NULL);
    startNode->definitely = listStatementNode;
  } else {
    startNode->definitely = endNode;
  }
  return startNode;
}

void initExceptions() {
  char **nodes = malloc(sizeof(char *) * START_ARRAY_SIZE);
  exceptions = (Array){START_ARRAY_SIZE, 0, nodes};
}

FunExecution *executionGraph(FilenameParseTree *input, int size) {
  FunExecution *result = malloc(sizeof(FunExecution) * size);
  for (int i = 0; i < size; ++i) {
    initExceptions();
    TreeNode *sourceNode = findSourceNode(input[i]);
    Array sourceItems = findSourceItems(sourceNode);
    for (int j = 0; j < sourceItems.nextPosition; ++j) {
      FunExecution currentFunExecution;
      currentFunExecution.filename = input[i].filename;
      TreeNode **currentSourceItemElements =
          ((TreeNode **)sourceItems.elements);
      currentFunExecution.name =
          currentSourceItemElements[j]->childNodes[0]->childNodes[0]->value;
      currentFunExecution.signature =
          currentSourceItemElements[j]->childNodes[0];
      currentFunExecution.nodes = initGraph(currentSourceItemElements[j]);
      currentFunExecution.errorsCount = exceptions.nextPosition;
      currentFunExecution.errors = exceptions.elements;
      result[size * i + j] = currentFunExecution;
    }
  }
  return result;
}
