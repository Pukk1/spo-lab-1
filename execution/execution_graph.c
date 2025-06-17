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

TreeNode *findThisObjectFirstArgumentOrNull(TreeNode *executeNode, FunCalls *funCalls) {
    TreeNode *readOrStaticFunctionNode = executeNode->childNodes[0];
    if (!strcmp(readOrStaticFunctionNode->type, "functionForCallName")) {
        return NULL;
    } else {
        TreeNode *objectMemberPlaceLinkNode = readOrStaticFunctionNode->childNodes[0];
        TreeNode *readObjectPlaceLinkNode = objectMemberPlaceLinkNode->childNodes[0];
        return operationTreeNode(readObjectPlaceLinkNode, funCalls);
    }
}

// для построения дерева операций
TreeNode *operationTreeNode(TreeNode *parsingTree, FunCalls *funCalls) {
    TreeNode *node = NULL;

    if (!strcmp(parsingTree->type, "functionForCallName")) {
        node = mallocTreeNode("FUNCTION_FOR_CALL_NAME", parsingTree->value, 0);
    } else if (!strcmp(parsingTree->type, "readPlace")) {
        node = mallocTreeNode("READ", NULL, 1);
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);
    } else if (!strcmp(parsingTree->type, "localPlaceLink")) {
        node = mallocTreeNode("LOCAL_PLACE_LINK", parsingTree->value, 0);
    } else if (!strcmp(parsingTree->type, "objectMember")) {
        node = mallocTreeNode("OBJECT_MEMBER", parsingTree->value, 0);
    } else if (!strcmp(parsingTree->type, "SET")) {
        node = mallocTreeNode("SET", NULL, 2);
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);
        node->childNodes[1] = operationTreeNode(parsingTree->childNodes[1], funCalls);
    } else if (!strcmp(parsingTree->type, "objectMemberPlaceLink")) {
        node = mallocTreeNode("OBJECT_MEMBER_PLACE_LINK", NULL, 2);
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);
        node->childNodes[1] = operationTreeNode(parsingTree->childNodes[1], funCalls);
    } else if (!strcmp(parsingTree->type, "indexPlaceLink")) {
        node = mallocTreeNode("INDEX_PLACE_LINK", NULL, 2);
        node->childNodes[0] = operationTreeNode(parsingTree->childNodes[0], funCalls);
        node->childNodes[1] = operationTreeNode(parsingTree->childNodes[1], funCalls);
    } else if (!strcmp(parsingTree->type, "call")) {
        TreeNode *thisObjectForFunCallOrNull = findThisObjectFirstArgumentOrNull(parsingTree, funCalls);
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

//        funcalls
        char funCallOperationIdNodeString[1024];
        sprintf(funCallOperationIdNodeString, "%d", node->id);
        TreeNode *funCallOperationIdNode = mallocTreeNode("operationTreeId", funCallOperationIdNodeString, 1);
        TreeNode *calledFunNameNode = mallocTreeNode("call", parsingTree->childNodes[0]->value, 0);
        funCallOperationIdNode->childNodes[0] = calledFunNameNode;
        addToList(funCalls->funCalls, funCallOperationIdNode);
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
    } else if (parsingTree->childrenNumber == 2) {
//        бинарные операции + set + listExpr
        node = mallocTreeNode(parsingTree->type, parsingTree->value, 2);
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
        } else if (!strcmp(parsingTree->type, "LABEL")) {
            typeByLiteral = "label";
        }
        char constVal[1024];
        sprintf(constVal, "%s", typeByLiteral);
        node->childNodes[0] = mallocTreeNode(NULL, typeByLiteral, 0);
        node->childNodes[1] = mallocTreeNode(NULL, parsingTree->value, 0);
    }
    return node;
}

//чтобы сгенерировать название для ноды
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

// декларация тела функции
ExecutionNode *listStatementExecutionNode(TreeNode *funcDefNode, ExecutionNode *nextNode, FunCalls *funCalls) {
    ExecutionNode *listStatements = nextNode;
    if (funcDefNode->childrenNumber == 2) {
        listStatements = executionNode(funcDefNode->childNodes[1], nextNode, NULL, funCalls);
    }
    return listStatements;
}

// декларация аргументов функции
ExecutionNode *functionArgsExecutionNode(TreeNode *functionSignatureNode, ExecutionNode *nextNode) {
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

ExecutionNode *initGraph(TreeNode *funcDefNode, FunCalls *funCalls) {
    ExecutionNode *startNode = initExecutionNode("START");
    ExecutionNode *endNode = initExecutionNode("FINISH");
    TreeNode *funcSignatureNode = funcDefNode->childNodes[0];

    ExecutionNode *listStatements = listStatementExecutionNode(funcDefNode, endNode, funCalls);
    ExecutionNode *functionArgs = functionArgsExecutionNode(funcSignatureNode, listStatements);

    startNode->definitely = functionArgs;
    return startNode;
}

FunExecution *initFunExecution(char *filename, bool isMethod, ExecutionNode *nodes, TreeNode *funCalls, char *funName) {
    FunExecution *funExecution = malloc(sizeof(FunExecution));
    funExecution->filename = filename;
    funExecution->isMethod = isMethod;
    funExecution->nodes = nodes;
    funExecution->funCalls = funCalls;
    funExecution->name = mallocString(funName);
    return funExecution;
}

FunExecution *funExecutionGraph(char *filename, TreeNode *funcDefNode, bool isMethod) {
    TreeNode *funcSignatureNode = funcDefNode->childNodes[0];
    char *funName = funcSignatureNode->value;

    List *functions = mallocEmptyList();
    FunCalls funCalls = (FunCalls) {functions, funName};

    ExecutionNode *nodes = initGraph(funcDefNode, &funCalls);

    TreeNode *funCallsRoot = mallocTreeNode("currentFunction", funName, funCalls.funCalls->size);
    for (int k = 0; k < funCalls.funCalls->size; ++k) {
        funCallsRoot->childNodes[k] = funCalls.funCalls->elements[k];
    }
    return initFunExecution(filename, isMethod, nodes, funCallsRoot, funName);
}

TreeNode *initFuncDefNode(char *funName, bool hasMembers) {
    int childNumber = 1;
    if (hasMembers) {
        childNumber = 2;
    }
    TreeNode *funcDefNode = mallocTreeNode("funcDef", NULL, childNumber);
    TreeNode *funcSignatureNode = mallocTreeNode("funcSignature", funName, 0);
    funcDefNode->childNodes[0] = funcSignatureNode;
    return funcDefNode;
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

TreeNode *initDimMembersNode() {
    TreeNode *varNode = mallocTreeNode("var", NULL, 2);
    TreeNode *listVarNode = mallocTreeNode("listVar", NULL, 1);
    varNode->childNodes[0] = listVarNode;
    TreeNode *varIdentifierNode = mallocTreeNode("IDENTIFIER", "members", 0);
    listVarNode->childNodes[0] = varIdentifierNode;
    TreeNode *typeDefNode = mallocTreeNode("TYPEDEF", "string", 0);
    varNode->childNodes[1] = typeDefNode;
    return varNode;
}

TreeNode *initMembersStartValueNode(int membersNumber) {
    TreeNode *membersSetNode = mallocTreeNode("SET", NULL, 2);
    TreeNode *localPlaceLinkNode = mallocTreeNode("localPlaceLink", "members", 0);
    membersSetNode->childNodes[0] = localPlaceLinkNode;
    TreeNode *strNode = mallocTreeNode("STR", createSpacesString(membersNumber), 0);
    membersSetNode->childNodes[1] = strNode;
    return membersSetNode;
}

TreeNode *initSetMembersStartArrayNode(int membersNumber) {
    TreeNode *setMembersValueNode = mallocTreeNode("SET", NULL, 2);
    TreeNode *membersPlaceLinkNode = mallocTreeNode("localPlaceLink", "members", 0);
    setMembersValueNode->childNodes[0] = membersPlaceLinkNode;
    TreeNode *membersStartValueNode = initMembersStartValueNode(membersNumber);
    setMembersValueNode->childNodes[1] = membersStartValueNode;
    return setMembersValueNode;
}

char *fieldName(TreeNode *classMemberNode) {
    TreeNode *classMemberDefNode = classMemberNode->childNodes[0];
    TreeNode *varNode = classMemberDefNode->childNodes[0];
    TreeNode *listVarNode = varNode->childNodes[0];
    TreeNode *fieldIdentifierNode = listVarNode->childNodes[0];
    return fieldIdentifierNode->value;
}

char *funcName(TreeNode *classMemberNode) {
    TreeNode *classMemberDefNode = classMemberNode->childNodes[1];
    TreeNode *funcDefNode = classMemberDefNode->childNodes[0];
    TreeNode *funcSignatureNode = funcDefNode->childNodes[0];
    return funcSignatureNode->value;
}

bool classMemberIsField(TreeNode *classMemberNode) {
    TreeNode *classMemberDefNode = classMemberNode->childNodes[0];
    bool isField = false;
    if (!strcmp(classMemberDefNode->value, "field")) {
        isField = true;
    }
    return isField;
}

char *classMemberName(TreeNode *classMemberNode) {
    char *memberName = NULL;
    if (classMemberIsField(classMemberNode)) {
        memberName = fieldName(classMemberNode);
    } else {
        memberName = funcName(classMemberNode);
    }
    return memberName;
}

bool classMemberIsPublic(TreeNode *classMemberNode) {
    TreeNode *classMemberModifierNode = classMemberNode->childNodes[0];
    bool isPublic = false;
    if (!strcmp(classMemberModifierNode->value, "public")) {
        isPublic = true;
    }
    return isPublic;
}

TreeNode *initMemberIndexPlaceLinkNode(int memberNumber) {
    TreeNode *indexPlaceLinkNode = mallocTreeNode("indexPlaceLink", NULL, 2);
    TreeNode *readPlaceNode = mallocTreeNode("readPlace", NULL, 1);
    indexPlaceLinkNode->childNodes[0] = readPlaceNode;
    TreeNode *localPlaceLinkNode = mallocTreeNode("localPlaceLink", "members", 0);
    readPlaceNode->childNodes[0] = localPlaceLinkNode;

    char memberNumberStr[10] = "";
    sprintf(memberNumberStr, "%d", memberNumber);
    TreeNode *decIndexNode = mallocTreeNode("DEC", memberNumberStr, 0);
    indexPlaceLinkNode->childNodes[1] = decIndexNode;
    return indexPlaceLinkNode;
}

TreeNode *initObjectMemberCallNode(TreeNode *classMemberNode) {
    TreeNode *callNode = mallocTreeNode("call", NULL, 2);
    TreeNode *functionForCallNameNode = mallocTreeNode("functionForCallName", "init_object_member", 0);
    callNode->childNodes[0] = functionForCallNameNode;

    TreeNode *memberNameListExprNode = initListExprNode(
            mallocTreeNode("STR", classMemberName(classMemberNode), 0),
            callNode,
            true
    );
    TreeNode *memberValueListExprNode = initListExprNode(
            mallocTreeNode("DEC", "0", 0),
            memberNameListExprNode,
            true
    );
    char *isPublicAsDec = "0";
    if (classMemberIsPublic(classMemberNode)) {
        isPublicAsDec = "1";
    }
    initListExprNode(
            mallocTreeNode("DEC", isPublicAsDec, 0),
            memberValueListExprNode,
            false
    );
    return callNode;
}

TreeNode *initClassMemberStatementNode(TreeNode *classMemberNode, int memberNumber) {
    TreeNode *setNode = mallocTreeNode("SET", NULL, 2);
    setNode->childNodes[0] = initMemberIndexPlaceLinkNode(memberNumber);
    setNode->childNodes[1] = initObjectMemberCallNode(classMemberNode);
    return setNode;
}

TreeNode *initReadMembersStatementNode() {
    TreeNode *readPlaceNode = mallocTreeNode("readPlace", NULL, 1);
    TreeNode *localPlaceLinkNode = mallocTreeNode("localPlaceLink", "members", 0);
    readPlaceNode->childNodes[0] = localPlaceLinkNode;
    return readPlaceNode;
}

// создание функции конструктора для класса
TreeNode *initConstructorFunNode(char *funName, List classMembers) {
    bool hasMembers = classMembers.size > 0;
    TreeNode *funcDefNode = initFuncDefNode(funName, hasMembers);
    TreeNode *dimMembersListStatementNode = initListStatementNode(
            initDimMembersNode(),
            funcDefNode,
            true
    );
    TreeNode *setMembersListStatementNode = initListStatementNode(
            initSetMembersStartArrayNode(classMembers.size),
            dimMembersListStatementNode,
            true
    );
    TreeNode *previousListStatement = setMembersListStatementNode;
    for (int memberNumber = 0; memberNumber < classMembers.size; ++memberNumber) {
        TreeNode *classMemberNode = classMembers.elements[memberNumber];
        previousListStatement = initListStatementNode(
                initClassMemberStatementNode(classMemberNode, memberNumber),
                previousListStatement,
                true
        );
    }
    initListStatementNode(initReadMembersStatementNode(), previousListStatement, false);
    return funcDefNode;
}

FunExecution *classExecutionsGraphs(char *filename, TreeNode *classDefNode, List *funExecutions) {
    List classMembers = *mallocEmptyList();
    if (classDefNode->childrenNumber > 1) {
        TreeNode *classMembersListNode = classDefNode->childNodes[1];
        classMembers = findListItemsUtil(classMembersListNode);
    }
    for (int i = 0; i < classMembers.size; ++i) {
        TreeNode *classMemberNode = classMembers.elements[i];
        TreeNode *classMemberTypeNode = classMemberNode->childNodes[1];
        char *classMemberType = classMemberTypeNode->type;
        if (!strcmp(classMemberType, "classFunc")) {
            TreeNode *funcDefNode = classMemberTypeNode->childNodes[0];
            FunExecution *methodExecution = funExecutionGraph(filename, funcDefNode, true);
            addToList(funExecutions, methodExecution);
        }
    }

    TreeNode *classSignatureNode = classDefNode->childNodes[0];
    char *className = classSignatureNode->value;
    TreeNode *constructorFunNode = initConstructorFunNode(className, classMembers);
    return funExecutionGraph(filename, constructorFunNode, false);
}

List *executionGraph(FilenameParseTree *input, int size) {
    List *funExecutions = mallocEmptyList();

    for (int i = 0; i < size; ++i) {
        FilenameParseTree currentFileParseTree = input[i];
        List sourceItems = findSourceItems(findSourceNode(currentFileParseTree));
        for (int j = 0; j < sourceItems.size; ++j) {
            TreeNode *sourceItem = sourceItems.elements[j];
            TreeNode *sourceItemDefNode = sourceItem->childNodes[0];
            FunExecution *funExecution;
            if (!strcmp(sourceItemDefNode->type, "funcDef")) {
                funExecution = funExecutionGraph(currentFileParseTree.filename, sourceItemDefNode, false);
            } else if (!strcmp(sourceItemDefNode->type, "classDef")) {
                funExecution = classExecutionsGraphs(currentFileParseTree.filename, sourceItemDefNode, funExecutions);
            } else {
                printException(sourceItemDefNode->type);
            }
            addToList(funExecutions, funExecution);
        }
    }
    return funExecutions;
}
