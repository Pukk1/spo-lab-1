#include "main.h"
#include "execution.h"
#include "node.h"
#include "parser.h"
#include <stdio.h>
#include <dirent.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
    if (argc > 2) {
        DIR *outputDir = opendir(argv[1]);
        if (!outputDir) {
            return 1;
        }
        FILE *inputFiles;
        for (int i = 2; i < argc; ++i) {
            inputFiles = fopen(argv[i], "r");
            if (!inputFiles) {
                return 1;
            }
        }
        for (int i = 0; i < argc - 2; ++i) {
            char *filename = basename(argv[i + 2]);
            FILE *inputFile = &inputFiles[i];
            char outputParseTreeFileName[1024];
            sprintf(outputParseTreeFileName, "%s/%s-parse-tree.txt", argv[1], basename(filename));
            FILE *outputParseTreeFile = fopen(outputParseTreeFileName, "w");
            ParseResult *result = parse(inputFile);
            printTree(result->nodes, result->size, outputParseTreeFile);
            Array *executionRes =
                    executionGraph(&(FilenameParseTree) {filename, result}, 1);
            for (int j = 0; j < result->errorsCount; ++j) {
                fprintf(stderr, "%s", result->errors[j]);
            }
            for (int j = 0; j < executionRes->nextPosition; ++j) {
                FunExecution *funExecution = executionRes->elements[j];
                for (int k = 0; k < funExecution->errorsCount; ++k) {
                    fprintf(stderr, "%s", funExecution->errors[k]);
                }
            }
            for (int j = 0; j < executionRes->nextPosition; ++j) {
                FunExecution *funExecution = executionRes->elements[j];

                char outputFunCallFileName[1024];
                sprintf(outputFunCallFileName, "%s/%s.%s.ext-fun-call.txt", argv[1], basename(funExecution->filename),
                        funExecution->name);
                FILE *outputFunCallFile = fopen(outputFunCallFileName, "w");
                char outputOperationTreesFileName[1024];
                sprintf(outputOperationTreesFileName, "%s/%s.%s.ext-operation-tree.txt", argv[1],
                        basename(funExecution->filename), funExecution->name);
                FILE *outputOperationTreesFile = fopen(outputOperationTreesFileName, "w");
                char outputExecutionFileName[1024];
                sprintf(outputExecutionFileName, "%s/%s.%s.ext-operation-tree.txt", argv[1],
                        basename(funExecution->filename), funExecution->name);
                FILE *outputExecutionFile = fopen(outputOperationTreesFileName, "w");

                printExecution(funExecution, outputFunCallFile, outputOperationTreesFile, outputExecutionFile);

                fclose(outputFunCallFile);
                fclose(outputOperationTreesFile);
                fclose(outputExecutionFile);
            }
            fclose(outputParseTreeFile);
            freeMem(result);
        }

        closedir(outputDir);
        for (int i = 0; i < argc - 2; ++i) {
            fclose(&inputFiles[i]);
        }
    } else {
        printf("Use parameter: with output dir and input files");
    }
    return 0;
}
