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

ExecutionNode *executionNode(TreeNode *treeNode, ExecutionNode *nextNode,
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
        return (Array) {0, 0, NULL};
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
ExecutionNode *executionListStatementNode(TreeNode *treeNode,
                                          ExecutionNode *nextNode,
                                          ExecutionNode *breakNode) {
    ExecutionNode *tmpNextNode = nextNode;
    if (treeNode->childrenNumber == 2) {
        tmpNextNode = executionNode(treeNode->childNodes[1], nextNode, breakNode);
    }
    ExecutionNode *node = initExecutionNode("");
    node->definitely =
            executionNode(treeNode->childNodes[0], tmpNextNode, breakNode);
    return node;
}

ExecutionNode *executionVarNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    Array variablesList = {0, 0, NULL};
    TreeNode *typeNode = NULL;
    if (treeNode->childrenNumber == 2) {
        typeNode = treeNode->childNodes[1];
        variablesList = findListItemsUtil(treeNode->childNodes[0]);
    } else {
        typeNode = treeNode->childNodes[0];
    }
    char resultNodeType[1024];
    if (!strcmp(typeNode->type, "array")) {
        int size = 0;
        if (typeNode->childrenNumber == 2) {
            size = findListItemsUtil(typeNode->childNodes[1]).nextPosition;
        }
        sprintf(resultNodeType,
                "array of %s size %d",
                typeNode->value, size);
    } else {
        sprintf(resultNodeType, "%s", typeNode->value);
    }

    ExecutionNode *previous = node;
    for (int i = 0; i < variablesList.nextPosition; ++i) {
        char varNameAndType[1024];
        sprintf(varNameAndType,
                "%s as %s",
                ((TreeNode *) variablesList.elements[i])->value, resultNodeType);
        previous->definitely = initExecutionNode(varNameAndType);
        previous = previous->definitely;
    }
    previous->definitely = nextNode;
    return node;
}

ExecutionNode *executionExpressionNode(TreeNode *treeNode, ExecutionNode *nextNode, ExecutionNode *conditionallyNext) {
    ExecutionNode *node = initExecutionNode("expressions");
    node->definitely = nextNode;
    node->conditionally = conditionallyNext;
//    TODO
    return node;
}

ExecutionNode *executionElseNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                 ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    if (treeNode->childrenNumber == 1) {
        node->definitely = executionNode(treeNode->childNodes[0],
                                         nextNode, breakNode);
    } else {
        node->definitely = nextNode;
    }
    return node;
}

ExecutionNode *executionIfNode(TreeNode *treeNode, ExecutionNode *nextNode,
                               ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    TreeNode *elseTreeNode = NULL;
    TreeNode *ifStatements = NULL;
    if (treeNode->childrenNumber == 3) {
        //    случай когда есть стейтменты в if и существует else
        ifStatements = treeNode->childNodes[1];
        elseTreeNode = treeNode->childNodes[2];
    } else if (treeNode->childrenNumber == 2 &&
               !strcmp(treeNode->childNodes[1]->type, "else")) {
        //    когда нет стейтментов в if
        elseTreeNode = treeNode->childNodes[1];
    } else if (treeNode->childrenNumber == 2) {
        ifStatements = treeNode->childNodes[1];
    }

    ExecutionNode *conditionNextNode = NULL;
    ExecutionNode *conditionConditionallyNode = NULL;
    if (elseTreeNode != NULL) {
        ExecutionNode *elseNode =
                executionElseNode(elseTreeNode, nextNode, breakNode);
        conditionNextNode = elseNode;
    } else {
        conditionNextNode = nextNode;
    }
    if (ifStatements != NULL) {
        ExecutionNode *statementsNode =
                executionNode(ifStatements, nextNode, breakNode);
        conditionConditionallyNode = statementsNode;
    }
    ExecutionNode *ifConditionNode = executionExpressionNode(treeNode->childNodes[0], conditionNextNode,
                                                             conditionConditionallyNode);
    node->definitely = ifConditionNode;

    return node;
}

ExecutionNode *executionWhileNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                  ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    ExecutionNode *statementNode = NULL;
    if (treeNode->childrenNumber == 2) {
        statementNode = executionNode(treeNode->childNodes[1], node, nextNode);
    } else {
        statementNode = initExecutionNode("");
        statementNode->definitely = node;
    }
    ExecutionNode *whileConditionNode = executionExpressionNode(treeNode->childNodes[0], nextNode,
                                                                statementNode);
    node->definitely = whileConditionNode;
    return node;
}

ExecutionNode *executionDoNode(TreeNode *treeNode, ExecutionNode *nextNode,
                               ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    TreeNode *conditionTreeNode = NULL;
    if (treeNode->childrenNumber == 3) {
        conditionTreeNode = treeNode->childNodes[2];
    } else {
        conditionTreeNode = treeNode->childNodes[1];
    }
    ExecutionNode *doConditionNode = executionExpressionNode(conditionTreeNode, nextNode, node);
    ExecutionNode *statementNode = NULL;
    if (treeNode->childrenNumber == 3) {
        statementNode = executionNode(treeNode->childNodes[0], doConditionNode, nextNode);
    } else {
        statementNode = initExecutionNode("");
        statementNode->definitely = doConditionNode;
    }
    node->definitely = statementNode;
    return node;
}

ExecutionNode *executionBreakNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                  ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    if (breakNode == NULL) {
        char exceptionText[1024];
        sprintf(exceptionText,
                "Exception in BREAK tree node parsing id --> %d no loop for break found",
                treeNode[0].id);
        addException(exceptionText);
        ExecutionNode *exceptionNode = initExecutionNode(exceptionText);
        node->definitely = exceptionNode;
        exceptionNode->definitely = nextNode;
    } else {
        node->definitely = breakNode;
    }
    return node;
}

// созадние блока
ExecutionNode *executionNode(TreeNode *treeNode, ExecutionNode *nextNode,
                             ExecutionNode *breakNode) {
    if (!strcmp(treeNode[0].type, "listStatement")) {
        return executionListStatementNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "var")) {
        return executionVarNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "if")) {
        return executionIfNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "while")) {
        return executionWhileNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "break")) {
        return executionBreakNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "do")) {
        return executionDoNode(treeNode, nextNode, breakNode);
    } else {
//        expression
        return executionExpressionNode(treeNode, nextNode, NULL);
    }
}

ExecutionNode *initGraph(TreeNode *sourceItem) {
    ExecutionNode *startNode = initExecutionNode("START");
    ExecutionNode *endNode = initExecutionNode("FINISH");
    TreeNode *funcDef = sourceItem->childNodes[0];
    if (funcDef->childrenNumber == 2) {
        ExecutionNode *listStatementNode =
                executionNode(funcDef->childNodes[1], endNode, NULL);
        startNode->definitely = listStatementNode;
    } else {
        startNode->definitely = endNode;
    }
    return startNode;
}

void initExceptions() {
    char **nodes = malloc(sizeof(char *) * START_ARRAY_SIZE);
    exceptions = (Array) {START_ARRAY_SIZE, 0, nodes};
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
                    ((TreeNode **) sourceItems.elements);
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
