//
// Created by Ivan on 14.06.2025.
//

#include "execution_util.h"

int currentExecutionId = -1;

int getNextExecutionId() {
    currentExecutionId++;
    return currentExecutionId;
}
