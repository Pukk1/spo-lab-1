//
// Created by Ivan on 26.01.2025.
//

#include "listing.h"
#include "execution.h"

void initListingParseNode(ExecutionNode *executionNode) {
    ListingNode *node = malloc(sizeof(ListingNode));
    node->node = executionNode;
    node->label = NULL;
    node->checked = 1;
    executionNode->listingNode = node;
}

void tryPlaceLabel(ExecutionNode *executionNode, int *labelCounter, bool necessaryLabeled) {
    if (executionNode == NULL) {
        return;
    }
    bool existListing = false;
    if (executionNode->listingNode != NULL) {
        existListing = true;
    } else {
        initListingParseNode(executionNode);
    }
    if (existListing || necessaryLabeled) {
        char *existingLabel = executionNode->listingNode->label;
        if (existingLabel == NULL) {
            ++(*labelCounter);
            char *label = malloc(sizeof(char) * 10);
            sprintf(label, "label%d", *labelCounter);
            executionNode->listingNode->label = label;
        }
    }
    if (!existListing) {
        tryPlaceLabel(executionNode->definitely, labelCounter, false);
        tryPlaceLabel(executionNode->conditionally, labelCounter, true);
    }
}

void placeLabels(Array *funExecutions) {
    int labelCounter = 0;
    for (int i = 0; i < funExecutions->nextPosition; ++i) {
        FunExecution *funExecution = funExecutions->elements[i];
        tryPlaceLabel(funExecution->nodes, &labelCounter, false);
    }
}

ValuePlaceAssociation *findValuePlace(Array *valuePlaceAssociations, char *name) {
    for (int i = 0; i < valuePlaceAssociations->nextPosition; ++i) {
        ValuePlaceAssociation *valuePlaceAssociation = valuePlaceAssociations->elements[0];
        if (!strcmp(valuePlaceAssociation->name, name)) {
            return valuePlaceAssociation;
        }
    }
    return NULL;
}

void printException(char *message) {
    printf("EXCEPTION: %s", message);
}

void fprintlnWithArg(char *message, char *arg, FILE *file) {
    fprintf(file, "%s %s\n", message, arg);
}

void fprintln(char *message, FILE *file) {
    fprintf(file, "%s\n", message);
}

ValuePlaceAssociation *addArgumentPlace(Array *valuePlaceAssociations, char *argName, char *argType) {
    ValuePlaceAssociation *findRes = findValuePlace(valuePlaceAssociations, argName);
    if (findRes != NULL) {
        char exceptionMessage[1000];
        sprintf(exceptionMessage, "duplicate argument name %s", findRes->name);
        printException(exceptionMessage);
        return findRes;
    } else {
        for (int i = 0; i < valuePlaceAssociations->nextPosition; ++i) {
            ValuePlaceAssociation *previousArg = valuePlaceAssociations->elements[i];
            previousArg->shiftPosition -= 2;
        }
        ValuePlaceAssociation *newArgAssociation = malloc(sizeof(ValuePlaceAssociation));
        newArgAssociation->name = argName;
        newArgAssociation->type = argType;
        newArgAssociation->shiftPosition = -2;
        addToList(valuePlaceAssociations, newArgAssociation);
        return newArgAssociation;
    }
}

ValuePlaceAssociation *addValuePlace(Array *valuePlaceAssociations, char *valuePlaceName, char *valuePlaceType) {
    ValuePlaceAssociation *findRes = findValuePlace(valuePlaceAssociations, valuePlaceName);
    if (findRes != NULL) {
        char exceptionMessage[1000];
        sprintf(exceptionMessage, "duplicate value place name %s", findRes->name);
        printException(exceptionMessage);
        return findRes;
    } else {
        ValuePlaceAssociation *newAssociation = malloc(sizeof(ValuePlaceAssociation));
        newAssociation->name = valuePlaceName;
        newAssociation->type = valuePlaceType;
        ValuePlaceAssociation *lastElement = NULL;
        if (valuePlaceAssociations->nextPosition > 0) {
            lastElement = valuePlaceAssociations->elements[valuePlaceAssociations->nextPosition - 1];
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

void tryPrintOperationTreeNode(TreeNode *operationTree, FILE *listingFile, Array *valuePlaceAssociations) {
    char *operationType = operationTree->type;
    if (!strcmp(operationType, "ARG")) {
        addArgumentPlace(
                valuePlaceAssociations,
                operationTree->childNodes[1]->value,
                operationTree->childNodes[0]->value
        );
    } else if (!strcmp(operationType, "AS")) {
        addValuePlace(
                valuePlaceAssociations,
                operationTree->childNodes[1]->value,
                operationTree->childNodes[0]->value
        );
        fprintln("PUSH 0", listingFile);
        fprintln("PUSH 0", listingFile);
    }
}

void tryPrintNode(ExecutionNode *executionNode, FILE *listingFile, Array *valuePlaceAssociations) {
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
        tryPrintOperationTreeNode(executionNode->operationTree, listingFile, valuePlaceAssociations);
        if (executionNode->conditionally != NULL) {
            fprintlnWithArg("JNZ", executionNode->conditionally->listingNode->label, listingFile);
        }
    }
    if (executionNode->definitely == NULL) {
        fprintln("RET", listingFile);
        return;
    }
    if (executionNode->definitely->listingNode->checked > 1) {
        fprintlnWithArg("JMP", executionNode->definitely->listingNode->label, listingFile);
    } else {
        tryPrintNode(executionNode->definitely, listingFile, valuePlaceAssociations);
    }
    if (executionNode->conditionally != NULL && executionNode->conditionally->listingNode->checked > 1) {
        return;
    } else {
        tryPrintNode(executionNode->conditionally, listingFile, valuePlaceAssociations);
    }
}

void printListing(Array *funExecutions, FILE *listingFile) {
    for (int i = 0; i < funExecutions->nextPosition; ++i) {
        Array *valuePlaceAssociationsArray = malloc(sizeof(Array));
        valuePlaceAssociationsArray->size = 100;
        valuePlaceAssociationsArray->nextPosition = 0;
        valuePlaceAssociationsArray->elements = malloc(sizeof(ValuePlaceAssociation) * 100);
        FunExecution *funExecution = funExecutions->elements[i];
        char funLabel[1000];
        sprintf(funLabel, "%s:", funExecution->name);
        fprintln(funLabel, listingFile);
        fprintln("PUSH 0", listingFile);
        fprintln("PUSH 0", listingFile);
        tryPrintNode(funExecution->nodes, listingFile, valuePlaceAssociationsArray);
        free(valuePlaceAssociationsArray);
    }
}