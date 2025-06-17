//
// Created by Ivan on 14.06.2025.
//

#include "execution_print.h"


void printNode(TreeNode *node, FILE *outputFile) {
    if (node == NULL) {
        return;
    }
    int childrenNumber = node->childrenNumber;
    for (int i = 0; i < childrenNumber; ++i) {
        printNode(node->childNodes[i], outputFile);
        fprintf(outputFile, "node%d", node->id);
        fprintf(outputFile, "([");
        int typeExists = 0;
        if (node->type != NULL && strlen(node->type) > 0) {
            fprintf(outputFile, "Type: %s", node->type);
            typeExists = 1;
        }
        if (node->value != NULL && strlen(node->value) > 0) {
            if (typeExists) {
                fprintf(outputFile, ", ", node->value);
            }
            fprintf(outputFile, "Value: %s", node->value);
        }
        fprintf(outputFile, "])");

        TreeNode *childNode = node->childNodes[i];
        fprintf(outputFile, " --> ");
        fprintf(outputFile, "node%d", childNode->id);
        fprintf(outputFile, "([");
        typeExists = 0;
        if (childNode->type != NULL && strlen(childNode->type) > 0) {
            fprintf(outputFile, "Type: %s", childNode->type);
            typeExists = 1;
        }
        if (childNode->value != NULL && strlen(childNode->value) > 0) {
            if (typeExists) {
                fprintf(outputFile, ", ");
            }
            fprintf(outputFile, "Value: %s", childNode->value);
        }
        fprintf(outputFile, "])");
        fprintf(outputFile, "\n");
    }
}

void printTreeNode(TreeNode *node, FILE *outputFile) {
    fprintf(outputFile, "flowchart TB\n");
    printNode(node, outputFile);
    fprintf(outputFile, "\n");
}

void printExecutionNode(ExecutionNode *father, ExecutionNode *child, FILE *outputFile, char *relationName) {
    fprintf(outputFile, "node%d", father->id);
    fprintf(outputFile, "([");
    fprintf(outputFile, "Text: %s", father->text);
    fprintf(outputFile, "])");
    fprintf(outputFile, " --%s--> ", relationName);
    fprintf(outputFile, "node%d", child->id);
    fprintf(outputFile, "([");
    fprintf(outputFile, "Text: %s", child->text);
    fprintf(outputFile, "])");
    fprintf(outputFile, "\n");
}

void printExecutionGraphNodeToFile(ExecutionNode *executionNode, FILE *outputOperationTreesFile,
                                   FILE *outputExecutionFile) {
    if (executionNode->printed) {
        return;
    } else {
        executionNode->printed = 1;
    }

    if (executionNode->operationTree) {
        char linkedExecutionNodeId[1024];
        sprintf(linkedExecutionNodeId, "%d", executionNode->id);
        TreeNode *linkedExecutionNode = mallocTreeNode("linked execution node id", linkedExecutionNodeId, 1);
        linkedExecutionNode->childNodes[0] = executionNode->operationTree;
        printNode(linkedExecutionNode, outputOperationTreesFile);
    }

    ExecutionNode *definitely = executionNode->definitely;
    if (definitely) {
        printExecutionGraphNodeToFile(definitely, outputOperationTreesFile, outputExecutionFile);

        printExecutionNode(executionNode, definitely, outputExecutionFile, "definitely");
    }

    ExecutionNode *conditionally = executionNode->conditionally;
    if (conditionally) {
        printExecutionGraphNodeToFile(conditionally, outputOperationTreesFile, outputExecutionFile);

        printExecutionNode(executionNode, conditionally, outputExecutionFile, "conditionally");
    }
}

void printExecutionGraphToFile(ExecutionNode *executionNode, FILE *outputOperationTreesFile,
                               FILE *outputExecutionFile) {
    fprintf(outputOperationTreesFile, "flowchart TB\n");
    fprintf(outputExecutionFile, "flowchart TB\n");
    printExecutionGraphNodeToFile(executionNode, outputOperationTreesFile, outputExecutionFile);
    fprintf(outputOperationTreesFile, "\n");
    fprintf(outputExecutionFile, "\n");
}

void printExecution(FunExecution *funExecution, FILE *outputFunCallFile, FILE *outputOperationTreesFile,
                    FILE *outputExecutionFile) {
    printTreeNode(funExecution->funCalls, outputFunCallFile);
    printExecutionGraphToFile(funExecution->nodes, outputOperationTreesFile, outputExecutionFile);
}