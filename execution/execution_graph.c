//
// Created by Ivan on 20.10.2024.
//

#include "execution_graph.h"

typedef struct FunCalls FunCalls;

struct FunCalls {
    List *funCalls;
    char *currentFunName;
};

ExecutionNode *executionNode(TreeNode *treeNode, ExecutionNode *nextNode,
                             ExecutionNode *breakNode, FunCalls *funCalls);

TreeNode *operationTreeNode(TreeNode *parsingTree, FunCalls *funCalls);


// получение корневого элемента из результатов парсинга
TreeNode *findSourceNode(FilenameParseTree input) {
    TreeNode **inputNodes = input.tree->nodes;
    int inputNodesSize = input.tree->size;
    TreeNode *sourceNode = inputNodes[inputNodesSize - 1];
    return sourceNode;
}

// получение списка функций из корневого элемента
List findSourceItems(TreeNode *source) {
    if (source->childrenNumber != 0) {
        TreeNode *sourceItemsList = source->childNodes[0];
        return findListItemsUtil(sourceItemsList);
    } else {
        return (List) {NULL, 0, 0};
    }
}

ExecutionNode *initExecutionNode(char *text) {
    ExecutionNode *node = malloc(sizeof(ExecutionNode));
    node->id = getNextExecutionId();
    node->text = mallocString(text);
    node->definitely = NULL;
    node->conditionally = NULL;
    node->operationTree = NULL;
    node->printed = 0;
    node->listingNode = NULL;
    return node;
}


// создание блока listStatement
ExecutionNode *executionListStatementNode(TreeNode *treeNode,
                                          ExecutionNode *nextNode,
                                          ExecutionNode *breakNode,
                                          FunCalls *funCalls) {
    ExecutionNode *tmpNextNode = nextNode;
    if (treeNode->childrenNumber == 2) {
        tmpNextNode = executionNode(treeNode->childNodes[1], nextNode, breakNode, funCalls);
    }
    ExecutionNode *node = initExecutionNode("");
    node->definitely =
            executionNode(treeNode->childNodes[0], tmpNextNode, breakNode, funCalls);
    return node;
}

ExecutionNode *executionVarNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                ExecutionNode *breakNode) {
    ExecutionNode *node = initExecutionNode("");
    List variablesList = {NULL, 0, 0};
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
            size = findListItemsUtil(typeNode->childNodes[1]).size;
        }
        sprintf(resultNodeType,
                "array of %s size %d",
                typeNode->value, size);
    } else {
        sprintf(resultNodeType, "%s", typeNode->value);
    }

    ExecutionNode *previous = node;
    for (int i = 0; i < variablesList.size; ++i) {
        char varNameAndType[1024];
        sprintf(varNameAndType,
                "AS %s %s",
                ((TreeNode *) variablesList.elements[i])->value, resultNodeType);
        ExecutionNode *initNode = initExecutionNode(varNameAndType);
        TreeNode *initOperationTreeNode = mallocTreeNode("AS", NULL, 2);
        initNode->operationTree = initOperationTreeNode;
        sprintf(varNameAndType,
                "%s",
                resultNodeType);
        initOperationTreeNode->childNodes[0] = mallocTreeNode(NULL, varNameAndType, 0);
        sprintf(varNameAndType,
                "%s",
                ((TreeNode *) variablesList.elements[i])->value);
        initOperationTreeNode->childNodes[1] = mallocTreeNode(NULL, varNameAndType, 0);
        previous->definitely = initNode;
        previous = previous->definitely;
    }
    previous->definitely = nextNode;
    return node;
}

TreeNode *findThisObjectFirstArgumentOrNull(TreeNode *placeChainTreeNode, FunCalls *funCalls) {
    TreeNode *readChainForField = operationTreeNode(placeChainTreeNode, funCalls);
    List readChainNodesList = findListItemsUtil(readChainForField);
    if (readChainNodesList.size == 1) {
        return NULL;
    } else {
        TreeNode *objectLinkForField = readChainForField;
        for (int i = 0; i < readChainNodesList.size - 2; ++i) {
            objectLinkForField = objectLinkForField->childNodes[1];
        }
        objectLinkForField->childrenNumber = 1;
        return readChainForField;
    }
}

// для построения дерева операций
TreeNode *operationTreeNode(TreeNode *parsingTree, FunCalls *funCalls) {
    TreeNode *node = NULL;

    if (!strcmp(parsingTree->type, "braces")) {
        return operationTreeNode(parsingTree->childNodes[0], funCalls);
    } else if (!strcmp(parsingTree->type, "variable")) {
        node = mallocTreeNode("READ_VAR", parsingTree->value, 0);
    } else if (!strcmp(parsingTree->type, "field")) {
        node = mallocTreeNode("READ_FIELD", parsingTree->value, 0);
    } else if (!strcmp(parsingTree->type, "placeChain")) {
        bool chainHasSeveralNodes = false;
        if (parsingTree->childrenNumber > 1) {
            chainHasSeveralNodes = true;
        }

        if (chainHasSeveralNodes) {
            node = mallocTreeNode("READ", NULL, 2);
            node->childNodes[1] = operationTreeNode(parsingTree->childNodes[1], funCalls);
        } else {
            node = mallocTreeNode("READ", NULL, 1);
        }
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);

    } else if (!strcmp(parsingTree->type, "INCREMENT") || !strcmp(parsingTree->type, "DECREMENT")) {
        node = mallocTreeNode("SET", NULL, 2);
        char valuePlace[1024];
        sprintf(valuePlace,
                "%s",
                parsingTree->childNodes[0]->value);
        node->childNodes[0] = mallocTreeNode(NULL, valuePlace, 0);
        if (!strcmp(parsingTree->type, "INCREMENT")) {
            node->childNodes[1] = mallocTreeNode("PLUS", NULL, 2);
        } else {
            node->childNodes[1] = mallocTreeNode("MINUS", NULL, 2);
        }
        node->childNodes[1]->childNodes[0] = mallocTreeNode(NULL, "const: 1", 0);
        node->childNodes[1]->childNodes[1] = operationTreeNode(parsingTree->childNodes[0], funCalls);
    } else if (!strcmp(parsingTree->type, "callOrIndexer")) {
        TreeNode *thisObjectForFunCallOrNull = findThisObjectFirstArgumentOrNull(parsingTree->childNodes[0], funCalls);
//        на загрузку самой функции
        int baseChildNodesNumber = 1;
//        на передачу this первым параметром функции, которая вызывается как метод объекта
        if (thisObjectForFunCallOrNull) {
            baseChildNodesNumber++;
        }
        if (parsingTree->childrenNumber == 1) {
            node = mallocTreeNode("EXECUTE", NULL, baseChildNodesNumber);
        } else {
            List argsList = findListItemsUtil(parsingTree->childNodes[1]);
            node = mallocTreeNode("EXECUTE", NULL, baseChildNodesNumber + argsList.size);
            for (int i = 0; i < argsList.size; ++i) {
                node->childNodes[i + baseChildNodesNumber] = operationTreeNode(argsList.elements[i], funCalls);
            }
        }
        if (thisObjectForFunCallOrNull) {
            node->childNodes[1] = thisObjectForFunCallOrNull;
        }
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);

        char funCallOperationIdNodeString[1024];
        sprintf(funCallOperationIdNodeString, "%d", node->id);
        TreeNode *funCallOperationIdNode = mallocTreeNode("operationTreeId", funCallOperationIdNodeString, 1);
        TreeNode *calledFunNameNode = mallocTreeNode("call", parsingTree->childNodes[0]->value, 0);
        funCallOperationIdNode->childNodes[0] = calledFunNameNode;
        addToList(funCalls->funCalls, funCallOperationIdNode);
    } else if (parsingTree->childrenNumber == 2) {
//        SET и ещё что-то
        node = mallocTreeNode(parsingTree->type, parsingTree->value, parsingTree->childrenNumber);
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);
        node->childNodes[1] = operationTreeNode(parsingTree->childNodes[1], funCalls);
    } else if (parsingTree->childrenNumber == 0) {
        node = mallocTreeNode("CONST", NULL, 2);
        char *typeByLiteral = "";
        if (!strcmp(parsingTree->type, "DEC")) {
            typeByLiteral = "int";
        } else if (!strcmp(parsingTree->type, "BIN")) {
            typeByLiteral = "int";
        } else if (!strcmp(parsingTree->type, "HEX")) {
            typeByLiteral = "int";
        } else if (!strcmp(parsingTree->type, "CHAR")) {
            typeByLiteral = "char";
        } else if (!strcmp(parsingTree->type, "STR")) {
            typeByLiteral = "str";
        } else if (!strcmp(parsingTree->type, "BOOL")) {
            typeByLiteral = "bool";
        }
        char constVal[1024];
        sprintf(constVal, "%s", typeByLiteral);
        node->childNodes[0] = mallocTreeNode(NULL, typeByLiteral, 0);
        node->childNodes[1] = mallocTreeNode(NULL, parsingTree->value, 0);
    }
    return node;
}

char *expressionNodeToString(TreeNode *treeNode) {
    if (treeNode->childrenNumber == 0) {
        return mallocString(treeNode->value);
    } else if (treeNode->childrenNumber == 1) {
        char *childStr = expressionNodeToString(treeNode->childNodes[0]);
        char text[1024];
        sprintf(text,
                "%s-%s-",
                treeNode->type, childStr);
        return mallocString(text);
    } else {
        char *childLeftStr = expressionNodeToString(treeNode->childNodes[0]);
        char *childRightStr = expressionNodeToString(treeNode->childNodes[1]);
        char text[1024];
        sprintf(text,
                "%s %s %s",
                childLeftStr, treeNode->type, childRightStr);
        return mallocString(text);
    }
}

ExecutionNode *executionExpressionNode(TreeNode *treeNode, FunCalls *funCalls) {
    ExecutionNode *node = initExecutionNode(expressionNodeToString(treeNode));
    node->operationTree = operationTreeNode(treeNode, funCalls);
    return node;
}

ExecutionNode *executionElseNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                 ExecutionNode *breakNode, FunCalls *funCalls) {
    ExecutionNode *node = initExecutionNode("");
    if (treeNode->childrenNumber == 1) {
        node->definitely = executionNode(treeNode->childNodes[0],
                                         nextNode, breakNode, funCalls);
    } else {
        node->definitely = nextNode;
    }
    return node;
}

ExecutionNode *executionIfNode(TreeNode *treeNode, ExecutionNode *nextNode,
                               ExecutionNode *breakNode, FunCalls *funCalls) {
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
                executionElseNode(elseTreeNode, nextNode, breakNode, funCalls);
        conditionNextNode = elseNode;
    } else {
        conditionNextNode = nextNode;
    }
    if (ifStatements != NULL) {
        ExecutionNode *statementsNode =
                executionNode(ifStatements, nextNode, breakNode, funCalls);
        conditionConditionallyNode = statementsNode;
    }
    ExecutionNode *conditionNode = executionExpressionNode(treeNode->childNodes[0], funCalls);
    node->definitely = conditionNode;
    conditionNode->definitely = conditionNextNode;
    conditionNode->conditionally = conditionConditionallyNode;
    return node;
}

ExecutionNode *executionWhileNode(TreeNode *treeNode, ExecutionNode *nextNode,
                                  ExecutionNode *breakNode, FunCalls *funCalls) {
    ExecutionNode *node = initExecutionNode("");
    ExecutionNode *statementNode = NULL;
    if (treeNode->childrenNumber == 2) {
        statementNode = executionNode(treeNode->childNodes[1], node, nextNode, funCalls);
    } else {
        statementNode = initExecutionNode("");
        statementNode->definitely = node;
    }
    ExecutionNode *conditionNode = executionExpressionNode(treeNode->childNodes[0], funCalls);
    node->definitely = conditionNode;
    conditionNode->definitely = nextNode;
    conditionNode->conditionally = statementNode;
    return node;
}

ExecutionNode *executionDoNode(TreeNode *treeNode, ExecutionNode *nextNode,
                               ExecutionNode *breakNode, FunCalls *funCalls) {
    ExecutionNode *node = initExecutionNode("");
    TreeNode *conditionTreeNode = NULL;
    if (treeNode->childrenNumber == 3) {
        conditionTreeNode = treeNode->childNodes[2];
    } else {
        conditionTreeNode = treeNode->childNodes[1];
    }
    ExecutionNode *doConditionNode = executionExpressionNode(conditionTreeNode, funCalls);
    doConditionNode->definitely = nextNode;
    doConditionNode->conditionally = node;
    ExecutionNode *statementNode = NULL;
    if (treeNode->childrenNumber == 3) {
        statementNode = executionNode(treeNode->childNodes[0], doConditionNode, nextNode, funCalls);
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
        printException(exceptionText);
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
                             ExecutionNode *breakNode, FunCalls *funCalls) {
    if (!strcmp(treeNode[0].type, "listStatement")) {
        return executionListStatementNode(treeNode, nextNode, breakNode, funCalls);
    } else if (!strcmp(treeNode[0].type, "var")) {
        return executionVarNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "if")) {
        return executionIfNode(treeNode, nextNode, breakNode, funCalls);
    } else if (!strcmp(treeNode[0].type, "while")) {
        return executionWhileNode(treeNode, nextNode, breakNode, funCalls);
    } else if (!strcmp(treeNode[0].type, "break")) {
        return executionBreakNode(treeNode, nextNode, breakNode);
    } else if (!strcmp(treeNode[0].type, "do")) {
        return executionDoNode(treeNode, nextNode, breakNode, funCalls);
    } else {
//        expression
        ExecutionNode *expressionNode = executionExpressionNode(treeNode, funCalls);
        expressionNode->definitely = nextNode;
    }
}

// декларация аргументов функции
ExecutionNode *functionArgsExecutionNode(TreeNode *functionSignatureNode, ExecutionNode *nextNode,
                                         ExecutionNode *breakNode, FunCalls *funCalls) {
    ExecutionNode *node = initExecutionNode("");
    node->definitely = nextNode;
    if (functionSignatureNode->childrenNumber > 0 &&
        !strcmp(functionSignatureNode->childNodes[0]->type, "listArgDef")) {
        List args = findListItemsUtil(functionSignatureNode->childNodes[0]);
        ExecutionNode *parentNode = node;
        for (int i = 0; i < args.size; ++i) {
            TreeNode *argDef = args.elements[i];
            char argText[1024];
            sprintf(argText, "ARG %s %s", argDef->childNodes[0]->value, argDef->childNodes[1]->value);
            ExecutionNode *newDimNode = initExecutionNode(argText);
            newDimNode->operationTree = mallocTreeNode("ARG", NULL, 2);
            newDimNode->operationTree->childNodes[0] = mallocTreeNode(NULL, argDef->childNodes[1]->value, 0);
            newDimNode->operationTree->childNodes[1] = mallocTreeNode(NULL, argDef->childNodes[0]->value, 0);
            newDimNode->definitely = parentNode->definitely;
            parentNode->definitely = newDimNode;
            parentNode = newDimNode;
        }
        parentNode->definitely = nextNode;
    }
    return node;
}

ExecutionNode *initGraph(TreeNode *sourceItem, FunCalls *funCalls) {
    ExecutionNode *startNode = initExecutionNode("START");
    ExecutionNode *endNode = initExecutionNode("FINISH");
    TreeNode *funcDef = sourceItem->childNodes[0];

    ExecutionNode *listStatements = endNode;
    if (funcDef->childrenNumber == 2) {
        listStatements =
                executionNode(funcDef->childNodes[1], endNode, NULL, funCalls);
    }
    ExecutionNode *functionArgs =
            functionArgsExecutionNode(funcDef->childNodes[0], listStatements, NULL, funCalls);
    startNode->definitely = functionArgs;
    return startNode;
}

SourceItemExecution *funExecutionGraph(char *filename, TreeNode *sourceItemElement, bool isMethod) {
    SourceItemExecution *sourceItemExecution = malloc(sizeof(SourceItemExecution));
    sourceItemExecution->isMethod = isMethod;
    sourceItemExecution->filename = filename;
    sourceItemExecution->name = sourceItemElement->childNodes[0]->childNodes[0]->value;

    List *functions = mallocEmptyList();
    FunCalls funCalls = (FunCalls) {functions, sourceItemExecution->name};
    sourceItemExecution->nodes = initGraph(sourceItemElement, &funCalls);
    TreeNode *funCallsRoot = mallocTreeNode("currentFunction", sourceItemExecution->name,
                                            funCalls.funCalls->size);
    for (int k = 0; k < funCalls.funCalls->size; ++k) {
        funCallsRoot->childNodes[k] = funCalls.funCalls->elements[k];
    }
    sourceItemExecution->funCalls = funCallsRoot;
    return sourceItemExecution;
}

//void classExecutionsGraphs(char *filename, TreeNode *sourceItemElement, Array *funExecutionsResultArray) {
//    SourceItemExecution *classExecution = malloc(sizeof(SourceItemExecution));
//    classExecution->filename = filename;
//    char classExecutionName[1024];
//    strcpy(classExecutionName, filename);
//    strcat(classExecutionName, sourceItemElement->childNodes[0]->childNodes[0]->value);
//    classExecution->name = mallocString(classExecutionName);
//}

List *executionGraph(FilenameParseTree *input, int size) {
    List *result = mallocEmptyList();

    for (int i = 0; i < size; ++i) {
        FilenameParseTree currentFileParseTree = input[i];
        List sourceItems = findSourceItems(findSourceNode(currentFileParseTree));
        for (int j = 0; j < sourceItems.size; ++j) {
            TreeNode *sourceItem = sourceItems.elements[j];
            if (!strcmp(sourceItem->childNodes[0]->type, "funcDef")) {
                void *sourceItemExecution = funExecutionGraph(currentFileParseTree.filename, sourceItem, false);
                addToList(result, sourceItemExecution);
            } else {
//                TODO
//                classExecutionsGraphs(currentFileParseTree.filename, sourceItem, result);
            }
        }
    }
    return result;
}
