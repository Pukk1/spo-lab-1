//
// Created by Ivan on 14.06.2025.
//

#ifndef SPO_LAB1_EXECUTION_PRINT_H
#define SPO_LAB1_EXECUTION_PRINT_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../execution_types.h"
#include "../../node/node_types.h"
#include "../../common/node/tree_node_util.h"


void printExecution(FunExecution *funExecution, FILE *outputFunCallFile, FILE *outputOperationTreesFile,
                    FILE *outputExecutionFile);

#endif //SPO_LAB1_EXECUTION_PRINT_H
