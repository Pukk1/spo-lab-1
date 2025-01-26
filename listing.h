//
// Created by Ivan on 26.01.2025.
//
#ifndef SPO_LAB1_LISTING_H
#define SPO_LAB1_LISTING_H

#include "main.h"

typedef struct ListingNode ListingNode;

struct ListingNode {
    ExecutionNode *node;
    char *label;
    int checked;
};

void placeLabels(Array *funExecutions);

void printListing(Array *funExecutions, FILE *listingFile);

#endif //SPO_LAB1_LISTING_H
