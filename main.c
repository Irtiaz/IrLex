#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "IrLexer.h"

void logTokenLexemePair(FILE *logFile, const char *token, const char *lexeme);

int main(int argc, char **argv) {
    FILE *inputFile;
    FILE *ruleFile;

    if (argc != 3) {
        fprintf(stderr, "Wrong usage! Sample use: <executable lexer> rules.txt code.txt\n");
        exit(1);
    }

    inputFile = fopen(argv[2], "r");
    ruleFile = fopen(argv[1], "r");

    if (!inputFile) {
        fprintf(stderr, "%s not found\n", argv[2]);
        exit(1);
    }

    if (!ruleFile) {
        fprintf(stderr, "%s not found\n", argv[1]);
        exit(1);
    }

    {
        IrLexer *irLexer = createIrLexer(ruleFile, inputFile);
        char token[MAX_TOKEN_CHARS];
        char lexeme[BUFFER_SIZE];
        do {
            getNextToken(irLexer, token, lexeme);
            logTokenLexemePair(stdout, token, lexeme);
        } while (strcmp(token, "$"));

        destroyIrLexer(irLexer);
    }

    fclose(inputFile);
    fclose(ruleFile);

    return 0;
}

void logTokenLexemePair(FILE *logFile, const char *token, const char *lexeme) {
    fprintf(logFile, "%s %s\n", token, lexeme);
}

