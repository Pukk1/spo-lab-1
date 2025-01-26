#include "main.h"


void printParseTree(char *outputDirName, FILE *inputFile, char *baseInputFileName, ParseResult *resultParseTree) {
    char outputParseTreeFileName[1024];
    sprintf(outputParseTreeFileName, "%s/%s-parse-tree.txt", outputDirName, baseInputFileName);
    FILE *outputParseTreeFile = fopen(outputParseTreeFileName, "w");
    printTree(resultParseTree->nodes, resultParseTree->size, outputParseTreeFile);
    for (int j = 0; j < resultParseTree->errorsCount; ++j) {
        fprintf(stderr, "%s", resultParseTree->errors[j]);
    }
    fclose(outputParseTreeFile);
}


void printExecutionGraph(char *outputDirName, char *baseInputFileName, ParseResult *resultParseTree,
                         Array *resultExecutionGraph) {
    for (int j = 0; j < resultExecutionGraph->nextPosition; ++j) {
        FunExecution *funExecution = resultExecutionGraph->elements[j];
        for (int k = 0; k < funExecution->errorsCount; ++k) {
            fprintf(stderr, "%s", funExecution->errors[k]);
        }
    }
    for (int j = 0; j < resultExecutionGraph->nextPosition; ++j) {
        FunExecution *funExecution = resultExecutionGraph->elements[j];

        char outputFunCallFileName[1024];
        sprintf(outputFunCallFileName, "%s/%s.%s.ext-fun-call.txt", outputDirName, basename(funExecution->filename),
                funExecution->name);
        FILE *outputFunCallFile = fopen(outputFunCallFileName, "w");
        char outputOperationTreesFileName[1024];
        sprintf(outputOperationTreesFileName, "%s/%s.%s.ext-operation-tree.txt", outputDirName,
                basename(funExecution->filename), funExecution->name);
        FILE *outputOperationTreesFile = fopen(outputOperationTreesFileName, "w");
        char outputExecutionFileName[1024];
        sprintf(outputExecutionFileName, "%s/%s.%s.ext", outputDirName,
                basename(funExecution->filename), funExecution->name);
        FILE *outputExecutionFile = fopen(outputExecutionFileName, "w");

        printExecution(funExecution, outputFunCallFile, outputOperationTreesFile, outputExecutionFile);

        fclose(outputFunCallFile);
        fclose(outputOperationTreesFile);
        fclose(outputExecutionFile);
    }
}

void printListingToFile(Array *resultExecutionGraph, char *outputDirName) {
    char outputListingFileName[1024];
    sprintf(outputListingFileName, "%s/listing.txt", outputDirName);
    FILE *outputParseTreeFile = fopen(outputListingFileName, "w");
    printListing(resultExecutionGraph, outputParseTreeFile);
    fclose(outputParseTreeFile);
}

void fileCloses(DIR *outputDir, FILE *inputFiles[], int inputFilesNumber) {
    closedir(outputDir);
    for (int i = 0; i < inputFilesNumber; ++i) {
        fclose(inputFiles[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc <= 2) {
        printf("Use parameter: with output dir and input files");
    }

    char *outputDirName = argv[1];
    DIR *outputDir = opendir(outputDirName);
    if (!outputDir) {
        printf("Can not open output dir");
        return 1;
    }

    int inputFilesNumber = argc - 2;
    char *inputFilesNames[inputFilesNumber];
    FILE *inputFiles[inputFilesNumber];
    for (int i = 0; i < inputFilesNumber; ++i) {
        char *fileName = argv[i + 2];
        inputFilesNames[i] = fileName;
        FILE *inputFile = fopen(fileName, "r");
        if (!inputFile) {
            printf("Can not open input file: %s", argv[i]);
            return 1;
        }
        inputFiles[i] = inputFile;
    }

    for (int i = 0; i < inputFilesNumber; ++i) {
        FILE *inputFile = inputFiles[i];
        char *baseInputFileName = basename(inputFilesNames[i]);

        ParseResult *resultParseTree = parse(inputFile);
        printParseTree(outputDirName, inputFile, baseInputFileName, resultParseTree);

        FilenameParseTree fileNameParseTree = (FilenameParseTree) {baseInputFileName, resultParseTree};
        Array *resultExecutionGraph = executionGraph(&fileNameParseTree, 1);
        printExecutionGraph(outputDirName, baseInputFileName, resultParseTree, resultExecutionGraph);

        placeLabels(resultExecutionGraph);
        printListingToFile(resultExecutionGraph, outputDirName);

        freeMem(resultParseTree);
    }

    fileCloses(outputDir, inputFiles, inputFilesNumber);
    return 0;
}
