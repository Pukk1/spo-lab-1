//
// Created by Ivan on 26.01.2025.
//

#include "listing.h"

ValuePlaceAssociation *findValuePlace(List *valuePlaceAssociations, char *name) {
    for (int i = 0; i < valuePlaceAssociations->size; ++i) {
        ValuePlaceAssociation *valuePlaceAssociation = valuePlaceAssociations->elements[i];
        if (!strcmp(valuePlaceAssociation->name, name)) {
            return valuePlaceAssociation;
        }
    }
    return NULL;
}

ValuePlaceAssociation *addArgumentPlace(List *valuePlaceAssociations, char *argName) {
    ValuePlaceAssociation *findRes = findValuePlace(valuePlaceAssociations, argName);
    if (findRes != NULL) {
        char exceptionMessage[1000];
        sprintf(exceptionMessage, "duplicate argument name %s", findRes->name);
        printException(exceptionMessage);
        return findRes;
    } else {
        for (int i = 0; i < valuePlaceAssociations->size; ++i) {
            ValuePlaceAssociation *previousArg = valuePlaceAssociations->elements[i];
            previousArg->shiftPosition -= 2;
        }
        ValuePlaceAssociation *newArgAssociation = malloc(sizeof(ValuePlaceAssociation));
        newArgAssociation->name = argName;
        newArgAssociation->shiftPosition = -2;
        addToList(valuePlaceAssociations, newArgAssociation);
        return newArgAssociation;
    }
}

ValuePlaceAssociation *addValuePlace(List *valuePlaceAssociations, char *valuePlaceName) {
    ValuePlaceAssociation *findRes = findValuePlace(valuePlaceAssociations, valuePlaceName);
    if (findRes != NULL) {
        char exceptionMessage[1000];
        sprintf(exceptionMessage, "duplicate value place name %s", findRes->name);
        printException(exceptionMessage);
        return findRes;
    } else {
        ValuePlaceAssociation *newAssociation = malloc(sizeof(ValuePlaceAssociation));
        newAssociation->name = valuePlaceName;
        ValuePlaceAssociation *lastElement = NULL;
        if (valuePlaceAssociations->size > 0) {
            lastElement = valuePlaceAssociations->elements[valuePlaceAssociations->size - 1];
        }
//        0 - адрес возврата, 1-2 возвращаемое значение
        int newElementShift = 3;
        if (lastElement != NULL && lastElement->shiftPosition > 0) {
            newElementShift = lastElement->shiftPosition + 2;
        }
        newAssociation->shiftPosition = newElementShift;
        addToList(valuePlaceAssociations, newAssociation);
        return newAssociation;
    }
}

void findFieldValuePlace() {

}

void tryPrintOperationTreeNode(TreeNode *operationTree, FILE *listingFile, List *valuePlaceAssociations,
                               int *argumentNumber) {
    char *operationType = operationTree->type;
    if (!strcmp(operationType, "ARG")) {
        (*argumentNumber)++;
        addArgumentPlace(valuePlaceAssociations, operationTree->childNodes[1]->value);
    } else if (!strcmp(operationType, "AS")) {
        addValuePlace(valuePlaceAssociations, operationTree->childNodes[1]->value);
        fprintln("PUSH 0", listingFile);
        fprintln("PUSH 0", listingFile);
    } else if (!strcmp(operationType, "FUNCTION_FOR_CALL_NAME")) {
        char *functionName = operationTree->value;
        fprintlnWithArg("PUSH", "4", listingFile);
        fprintlnWithArg("PUSH", functionName, listingFile);
    } else if (!strcmp(operationType, "CONST")) {
        char *constType = operationTree->childNodes[0]->value;
        char *constValue = operationTree->childNodes[1]->value;
        if (!strcmp(constType, "int")) {
            fprintlnWithArg("PUSH", "1", listingFile);
            fprintlnWithArg("PUSH", constValue, listingFile);
        } else if (!strcmp(constType, "char")) {
            char value[1];
            sprintf(value, "%d", (int) constValue[0]);
            fprintlnWithArg("PUSH", "3", listingFile);
            fprintlnWithArg("PUSH", value, listingFile);
        } else if (!strcmp(constType, "bool")) {
            if (!strcmp(constValue, "true")) {
                fprintlnWithArg("PUSH", "1", listingFile);
                fprintlnWithArg("PUSH", "1", listingFile);
            } else {
                fprintlnWithArg("PUSH", "1", listingFile);
                fprintlnWithArg("PUSH", "0", listingFile);
            }
        } else if (!strcmp(constType, "str")) {
            int valueStrSize = strlen(constValue);
            char alloc_size[30];
            sprintf(alloc_size, "%d", valueStrSize * 2 + 2);
            fprintlnWithArg("ALLOC", alloc_size, listingFile);
            for (int i = 0; i < valueStrSize; ++i) {
                char shift[30];
                sprintf(shift, "%d", 8 * 2 * i);
                fprintlnWithArg("PUSH", "1", listingFile);
                fprintlnWithArg("PUSH", shift, listingFile);
                fprintln("SUM", listingFile);
                char value[30];
                sprintf(value, "%d", (int) constValue[i]);
                fprintlnWithArg("PUSH", "3", listingFile);
                fprintlnWithArg("PUSH", value, listingFile);
//                возвращает индекс, в который записал значение
                fprintln("SAVE", listingFile);
//                возврат на адрес начала
                fprintlnWithArg("PUSH", "1", listingFile);
                fprintlnWithArg("PUSH", shift, listingFile);
                fprintln("SUB", listingFile);
            }
            char shift[30];
            sprintf(shift, "%d", 8 * 2 * valueStrSize);
            fprintlnWithArg("PUSH", "1", listingFile);
            fprintlnWithArg("PUSH", shift, listingFile);
            fprintln("SUM", listingFile);
            fprintlnWithArg("PUSH", "3", listingFile);
            fprintlnWithArg("PUSH", "0", listingFile);
            fprintln("SAVE", listingFile);
            fprintlnWithArg("PUSH", "1", listingFile);
            fprintlnWithArg("PUSH", shift, listingFile);
            fprintln("SUB", listingFile);

        } else if (!strcmp(constType, "label")) {
            fprintlnWithArg("PUSH", "4", listingFile);
            fprintlnWithArg("PUSH", constValue, listingFile);
        } else {
            printException(operationType);
        }
    } else if (!strcmp(operationType, "SET")) {
        TreeNode *placeNode = operationTree->childNodes[0];
        TreeNode *valueNode = operationTree->childNodes[1];
        tryPrintOperationTreeNode(placeNode, listingFile, valuePlaceAssociations,
                                  argumentNumber);
        tryPrintOperationTreeNode(valueNode, listingFile, valuePlaceAssociations,
                                  argumentNumber);
        fprintln("SAVE", listingFile);
    } else if (!strcmp(operationType, "READ")) {
        TreeNode *placeLink = operationTree->childNodes[0];
        tryPrintOperationTreeNode(placeLink, listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("LOAD", listingFile);
    } else if (!strcmp(operationType, "LOCAL_PLACE_LINK")) {
        TreeNode *localPlaceLink = operationTree;
        ValuePlaceAssociation *valuePlace = findValuePlace(valuePlaceAssociations, localPlaceLink->value);

        if (valuePlace == NULL) {
            char exceptionMessage[1000];
            sprintf(exceptionMessage, "value place not found by name %s", localPlaceLink->value);
            printException(exceptionMessage);
            return;
        }

        char valuePlaceShift[1000];
        sprintf(valuePlaceShift, "%d", valuePlace->shiftPosition);

        fprintlnWithArg("LOAD_BP", valuePlaceShift, listingFile);
        if (operationTree->childrenNumber > 1) {
            tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations,
                                      argumentNumber);
        }
    } else if (!strcmp(operationType, "INDEX_PLACE_LINK")) {
        TreeNode *indexPlaceLinkNode = operationTree;
        TreeNode *placeLinkForIndexingNode = indexPlaceLinkNode->childNodes[0];
        TreeNode *indexNode = indexPlaceLinkNode->childNodes[1];
        tryPrintOperationTreeNode(placeLinkForIndexingNode, listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(indexNode, listingFile, valuePlaceAssociations, argumentNumber);

        fprintlnWithArg("PUSH", "4", listingFile);
        fprintlnWithArg("PUSH", "16", listingFile);
        fprintln("MUL", listingFile);
        fprintln("SUM", listingFile);
    } else if (!strcmp(operationType, "EQUALITY")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("EQ", listingFile);
    } else if (!strcmp(operationType, "NOTEQUAL")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("NEQ", listingFile);
    } else if (!strcmp(operationType, "SUM")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("SUM", listingFile);
    } else if (!strcmp(operationType, "SUB")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("SUB", listingFile);
    } else if (!strcmp(operationType, "MUL")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("MUL", listingFile);
    } else if (!strcmp(operationType, "DIV")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("DIV", listingFile);
    } else if (!strcmp(operationType, "PERCENT")) {
        tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations, argumentNumber);
        tryPrintOperationTreeNode(operationTree->childNodes[1], listingFile, valuePlaceAssociations, argumentNumber);
        fprintln("MOD", listingFile);
    } else if (!strcmp(operationType, "EXECUTE")) {
        TreeNode *staticFunctionOrLinkNode = operationTree->childNodes[0];
        if (!strcmp(staticFunctionOrLinkNode->type, "FUNCTION_FOR_CALL_NAME") &&
            !strcmp(staticFunctionOrLinkNode->value, "stdin") &&
            !strcmp(staticFunctionOrLinkNode->value, "stdout")) {

            char *staticFunctionName = staticFunctionOrLinkNode->value;
            if (!strcmp(staticFunctionName, "stdin")) {
                fprintln("LOAD_IN", listingFile);
            } else {
                tryPrintOperationTreeNode(
                        operationTree->childNodes[1],
                        listingFile,
                        valuePlaceAssociations,
                        argumentNumber
                );
                fprintln("SAVE_OUT", listingFile);
            }
        } else {
//            аргументы
            for (int i = 1; i < operationTree->childrenNumber; ++i) {
                tryPrintOperationTreeNode(operationTree->childNodes[i], listingFile, valuePlaceAssociations,
                                          argumentNumber);
            }
//            ссылка на функцию
            tryPrintOperationTreeNode(operationTree->childNodes[0], listingFile, valuePlaceAssociations,
                                      argumentNumber);

            fprintln("CALL", listingFile);
        }
    } else {
        printException(operationType);
    }
}

void tryPrintNode(ExecutionNode *executionNode, FILE *listingFile, List *valuePlaceAssociations, int *argumentNumber) {
    if (executionNode == NULL) {
        return;
    }
    if (executionNode->listingNode->label != NULL) {
        char label[1000];
        sprintf(label, "%s:", executionNode->listingNode->label);
        fprintln(label, listingFile);
    }
    executionNode->listingNode->checked++;
    if (executionNode->operationTree != NULL) {
        tryPrintOperationTreeNode(executionNode->operationTree, listingFile, valuePlaceAssociations, argumentNumber);
        if (executionNode->conditionally != NULL) {
            fprintlnWithArg("JNZ", executionNode->conditionally->listingNode->label, listingFile);
        }
    }
    if (executionNode->definitely == NULL) {
        char argNumberString[10];
        sprintf(argNumberString, "%d", *argumentNumber);
        fprintln("SAVE_BP 1", listingFile);
        fprintlnWithArg("RET", argNumberString, listingFile);
        return;
    }
    if (executionNode->definitely->listingNode->checked > 1) {
        fprintlnWithArg("JMP", executionNode->definitely->listingNode->label, listingFile);
    } else {
        tryPrintNode(executionNode->definitely, listingFile, valuePlaceAssociations, argumentNumber);
    }
    if (executionNode->conditionally != NULL && executionNode->conditionally->listingNode->checked > 1) {
        return;
    } else {
        tryPrintNode(executionNode->conditionally, listingFile, valuePlaceAssociations, argumentNumber);
    }
}

void printListing(List *funExecutions, FILE *listingFile) {
    fprintln("[section ram]", listingFile);
    fprintln("INIT code_end_addr", listingFile);
    fprintln("PUSH 4", listingFile);
    fprintln("PUSH main", listingFile);
    fprintln("CALL", listingFile);
    fprintln("POP", listingFile);
    fprintln("HLT", listingFile);
    for (int i = 0; i < funExecutions->size; ++i) {
        SourceItemExecution *funExecution = funExecutions->elements[i];
        List *valuePlaceAssociations = malloc(sizeof(List));
        int argumentNumber = 0;
        if (funExecution->isMethod) {
//        this
            argumentNumber++;
            addArgumentPlace(valuePlaceAssociations, "this");
        }
        valuePlaceAssociations->capacity = 100;
        valuePlaceAssociations->size = 0;
        valuePlaceAssociations->elements = malloc(sizeof(ValuePlaceAssociation) * 100);
        char funLabel[1000];
        sprintf(funLabel, "%s:", funExecution->name);
        fprintln(funLabel, listingFile);
        fprintln("PUSH 0", listingFile);
        fprintln("PUSH 0", listingFile);
        tryPrintNode(funExecution->nodes, listingFile, valuePlaceAssociations, &argumentNumber);
        free(valuePlaceAssociations);
    }
    fprintln("code_end_addr:", listingFile);
}