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
    if (executionNode->listingNode != NULL || necessaryLabeled) {
        if (necessaryLabeled && executionNode->listingNode == NULL) {
            initListingParseNode(executionNode);
        }
        char *existingLabel = executionNode->listingNode->label;
        if (existingLabel == NULL) {
            ++(*labelCounter);
            char *label = malloc(sizeof(char) * 10);
            sprintf(label, "%d", *labelCounter);
            executionNode->listingNode->label = label;
        }
    } else {
        initListingParseNode(executionNode);
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

void tryPrintConditionNode() {

}

void tryPrintOperationTreeNode(ExecutionNode *executionNode, FILE *listingFile) {

}

void tryPrintNode(ExecutionNode *executionNode, FILE *listingFile) {
    if (executionNode == NULL) {
        return;
    }
    executionNode->listingNode->checked++;
    if (executionNode->definitely == NULL) {
        return;
    }
    if (executionNode->operationTree != NULL) {
        tryPrintOperationTreeNode(executionNode, listingFile);
    }
    if (executionNode->definitely->listingNode->checked > 1) {

    } else {
        tryPrintNode(executionNode->definitely, listingFile);
    }
}

void printListing(Array *funExecutions, FILE *listingFile) {
    for (int i = 0; i < funExecutions->nextPosition; ++i) {
        FunExecution *funExecution = funExecutions->elements[i];
        fprintf(listingFile, "%s:\n", funExecution->name);
        tryPrintNode(funExecution->nodes, listingFile);
    }
}