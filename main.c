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

void getNextToken(FILE *inputFile, LexRule **rules, char buffer[BUFFER_SIZE], char *lastReadCharacter, char token[MAX_TOKEN_CHARS], char lexeme[BUFFER_SIZE]);

int main(int argc, char **argv) {
    FILE *inputFile;
    FILE *ruleFile;

    LexRule **rules = NULL;
    char buffer[BUFFER_SIZE] = {0};

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
        char lastReadCharacter = fgetc(inputFile);
        char token[MAX_TOKEN_CHARS];
        char lexeme[BUFFER_SIZE];
        do {
            getNextToken(inputFile, rules, buffer, &lastReadCharacter, token, lexeme);
            logTokenLexemePair(stdout, token, lexeme);
        } while (strcmp(token, "$"));
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

void getNextToken(FILE *inputFile, LexRule **rules, char buffer[BUFFER_SIZE], char *lastReadCharacter, char token[MAX_TOKEN_CHARS], char lexeme[BUFFER_SIZE]) {
    int matchFlag = 0;
    int bufferIndex = 0;

    strcpy(token, "garbage");
    strcpy(lexeme, "garbage");

    if (*lastReadCharacter == EOF) {
        strcpy(token, "$");
        strcpy(lexeme, "$");
        return;
    }

    while (*lastReadCharacter != EOF) {
        int currentlyMatching;

        buffer[bufferIndex] = *lastReadCharacter;
        currentlyMatching = getMatchingLexRule(rules, buffer) != NULL;

        if (matchFlag && !currentlyMatching) {
            char *matchingToken;
            buffer[bufferIndex] = '\0';
            matchingToken = getMatchingLexRule(rules, buffer)->token;
            if (strcmp(matchingToken, SKIP_TOKEN)) {
                strcpy(token, matchingToken);
                strcpy(lexeme, buffer);
                memset(buffer, '\0', BUFFER_SIZE);
                return;
            }

            memset(buffer, '\0', BUFFER_SIZE);
            bufferIndex = 0;
        }
        else {
            ++bufferIndex;
            *lastReadCharacter = fgetc(inputFile);
        }

        matchFlag = currentlyMatching;
    }

    {
        LexRule *lastRule = getMatchingLexRule(rules, buffer);
        if (lastRule && strcmp(lastRule->token, SKIP_TOKEN)) {
            strcpy(token, lastRule->token);
            strcpy(lexeme, buffer);
        }
        else {
            strcpy(token, "$");
            strcpy(lexeme, "$");
        }
        return;
    }
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
