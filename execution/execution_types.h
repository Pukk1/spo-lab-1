//
// Created by Ivan on 14.06.2025.
//

#ifndef SPO_LAB1_EXECUTION_TYPES_H
#define SPO_LAB1_EXECUTION_TYPES_H

#include <stdbool.h>
#include "../node/node_types.h"
//#include "../listing/listing_types.h"

typedef struct ExecutionNode ExecutionNode;
typedef struct SourceItemExecution SourceItemExecution;
typedef struct ListingNode ListingNode;

struct ExecutionNode {
    char *text;
    ExecutionNode *definitely;    // безусловный переход
    ExecutionNode *conditionally; // условный переход
    TreeNode *operationTree;
    int id;
    int printed;
    ListingNode *listingNode;
};

struct SourceItemExecution {
    char *name;
    char *filename;
    TreeNode *funCalls;
    ExecutionNode *nodes;
    char **errors;
    int errorsCount;
    bool isMethod;
};

struct ListingNode {
    ExecutionNode *node;
    char *label;
    int checked;
};

#endif //SPO_LAB1_EXECUTION_TYPES_H
