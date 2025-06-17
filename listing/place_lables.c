//
// Created by Ivan on 14.06.2025.
//

#include "place_lables.h"

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

void placeLabels(List *funExecutions) {
    int labelCounter = 0;
    for (int i = 0; i < funExecutions->size; ++i) {
        FunExecution *funExecution = funExecutions->elements[i];
        tryPlaceLabel(funExecution->nodes, &labelCounter, false);
    }
}
