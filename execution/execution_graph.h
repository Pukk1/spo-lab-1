//
// Created by Ivan on 20.10.2024.
//

#ifndef SPO_LAB1_EXECUTION_GRAPH_H
#define SPO_LAB1_EXECUTION_GRAPH_H

#include "execution_types.h"
#include "../common/list/list_util.h"
#include "../parse/parse_types.h"
#include "../common/node/tree_node_util.h"
#include "../common/exception/exception_util.h"
#include <string.h>
#include <stdbool.h>
#include "../node/node.h"

typedef struct FilenameParseTree FilenameParseTree;


struct FilenameParseTree {
    char *filename;
    ParseResult *tree;
};

List *executionGraph(FilenameParseTree *input, int size);

#endif // SPO_LAB1_EXECUTION_GRAPH_H
