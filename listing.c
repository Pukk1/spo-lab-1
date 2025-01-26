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

void tryPrintOperationTreeNode(ExecutionNode *executionNode, FILE *listingFile) {
    fprintf(listingFile, "%s\n", executionNode->text);
    if (executionNode->conditionally != NULL) {
        fprintf(listingFile, "JNZ %s\n", executionNode->conditionally->listingNode->label);
    }
}

void tryPrintNode(ExecutionNode *executionNode, FILE *listingFile) {
    if (executionNode == NULL) {
        return;
    }
    if (executionNode->listingNode->label != NULL) {
        fprintf(listingFile, "%s:\n", executionNode->listingNode->label);
    }
    executionNode->listingNode->checked++;
    if (executionNode->operationTree != NULL) {
        tryPrintOperationTreeNode(executionNode, listingFile);
    }
    if (executionNode->definitely == NULL) {
        fprintf(listingFile, "%s\n", "RET");
        return;
    }
    if (executionNode->definitely->listingNode->checked > 1) {
        fprintf(listingFile, "%s %s\n", "JMP", executionNode->definitely->listingNode->label);
    } else {
        tryPrintNode(executionNode->definitely, listingFile);
    }
    if (executionNode->conditionally != NULL && executionNode->conditionally->listingNode->checked > 1) {
        return;
    } else {
        tryPrintNode(executionNode->conditionally, listingFile);
    }
}

void printListing(Array *funExecutions, FILE *listingFile) {
    for (int i = 0; i < funExecutions->nextPosition; ++i) {
        FunExecution *funExecution = funExecutions->elements[i];
        fprintf(listingFile, "%s:\n", funExecution->name);
        tryPrintNode(funExecution->nodes, listingFile);
    }
}