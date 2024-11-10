#include "main.h"
#include "execution.h"
#include "node.h"
#include "parser.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 1) {
        FILE *input_file = fopen(argv[1], "r");
        FILE *output_file = fopen(argv[2], "w");
        if (input_file && output_file) {
            ParseResult *result = parse(input_file);
            fclose(input_file);
            printTree(result->nodes, result->size, output_file);
            fclose(output_file);
            FunExecution *executionRes =
                    executionGraph(&(FilenameParseTree) {argv[1], result}, 1);
            for (int i = 0; i < result->errorsCount; ++i) {
                fprintf(stderr, result->errors[i]);
            }
            freeMem(result);
        } else {
            int illegalArgNumber;
            if (!input_file) {
                illegalArgNumber = 1;
            } else {
                illegalArgNumber = 2;
            }
            printf("Error opening file: %s\n", argv[illegalArgNumber]);
        }
    } else {
        printf("Use only two parameters with input and output files paths");
    }
    return 0;
}
