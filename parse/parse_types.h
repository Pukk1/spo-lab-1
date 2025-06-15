//
// Created by Ivan on 14.06.2025.
//

#ifndef SPO_LAB1_PARSE_TYPES_H
#define SPO_LAB1_PARSE_TYPES_H

#include "../node/node_types.h"

typedef struct ParseResult ParseResult;

struct ParseResult {
    int size;
    TreeNode **nodes;
};

#endif //SPO_LAB1_PARSE_TYPES_H
