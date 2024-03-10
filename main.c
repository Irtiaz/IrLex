#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define MAX_REGEX_CHARS 40
#define MAX_TOKEN_CHARS 20
#define BUFFER_SIZE 100

#define SKIP_TOKEN "SKIP"

int matchRegex(const regex_t *regex, const char *string);
regex_t *prepareRegex(const char *regexString);

typedef struct {
    char *token;
    regex_t *regex;
} LexRule;

LexRule *createLexRule(const char *tokenName, const char *regexString);
LexRule *getMatchingLexRule(LexRule **rules, const char *buffer);
void freeLexRule(LexRule *rule);

void logTokenLexemePair(FILE *logFile, const char *token, const char *lexeme);

int main(int argc, char **argv) {
    FILE *inputFile;
    FILE *ruleFile;

    LexRule **rules = NULL;
    char buffer[BUFFER_SIZE] = {0};
    int bufferIndex = 0;

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
    
    while (1) {
        char tokenName[MAX_TOKEN_CHARS];
        char regexString[MAX_REGEX_CHARS];
        int readFlag = fscanf(ruleFile, "%s %s", tokenName, regexString);
        if (readFlag == EOF) break;
        arrput(rules, createLexRule(tokenName, regexString));
    } 

    {
        int matchFlag = 0;
        char c = fgetc(inputFile);
        while (c != EOF) {
            int currentlyMatching;

            buffer[bufferIndex] = c;
            currentlyMatching = getMatchingLexRule(rules, buffer) != NULL;

            if (matchFlag && !currentlyMatching) {
                char *token;
                buffer[bufferIndex] = '\0';
                token = getMatchingLexRule(rules, buffer)->token;
                if (strcmp(token, SKIP_TOKEN)) logTokenLexemePair(stdout, token, buffer);

                bufferIndex = 0;
                memset(buffer, '\0', sizeof(buffer));
            }
            else {
                ++bufferIndex;
                c = fgetc(inputFile);
            }

            matchFlag = currentlyMatching;
        }
    }

    {
        LexRule *lastRule = getMatchingLexRule(rules, buffer);
        if (lastRule && strcmp(lastRule->token, SKIP_TOKEN)) logTokenLexemePair(stdout, lastRule->token, buffer);
    }

    {
        int i;
        for (i = 0; i < arrlen(rules); ++i) {
            freeLexRule(rules[i]);
        }
    }

    arrfree(rules);
    fclose(inputFile);
    fclose(ruleFile);

    return 0;
}

int matchRegex(const regex_t *regex, const char *string) {
    int matched = regexec(regex, string, 0, NULL, 0) != REG_NOMATCH;
    return matched;
}

regex_t *prepareRegex(const char *regexString) {
    char modifiedRegexString[MAX_REGEX_CHARS];
    regex_t *regex = (regex_t *)malloc(sizeof(regex_t));

    modifiedRegexString[0] = '^';

    {
        int i;
        for (i = 0; i < MAX_REGEX_CHARS && regexString[i]; ++i) {
            modifiedRegexString[i + 1] = regexString[i];
        }
        modifiedRegexString[++i] = '$';
        modifiedRegexString[++i] = '\0';
    }

    if (regcomp(regex, modifiedRegexString, 0)) {
        printf("Could not compile regex: %s\n", regexString); 
        exit(1);
    }

    return regex;
}

LexRule *createLexRule(const char *tokenName, const char *regexString) {
    LexRule *rule = (LexRule *)malloc(sizeof(LexRule));
    rule->token = (char *)malloc(strlen(tokenName) + 1);
    strcpy(rule->token, tokenName);
    rule->regex = prepareRegex(regexString);
    return rule;
}

LexRule *getMatchingLexRule(LexRule **rules, const char *buffer) {
    int i;
    for (i = 0; i < arrlen(rules); ++i) {
        if (matchRegex(rules[i]->regex, buffer)) return rules[i];
    }
    return NULL;
}


void freeLexRule(LexRule *rule) {
    regfree(rule->regex);
    free(rule->regex);
    free(rule->token);
    free(rule);
}

void logTokenLexemePair(FILE *logFile, const char *token, const char *lexeme) {
    fprintf(logFile, "%s %s\n", token, lexeme);
}
