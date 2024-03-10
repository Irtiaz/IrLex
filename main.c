#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define MAX_REGEX_CHARS 40
#define BUFFER_SIZE 100

int matchRegex(const regex_t *regex, const char *string);
regex_t *prepareRegex(const char *regexString);

typedef struct {
    const char *token;
    regex_t *regex;
} LexRule;

LexRule *createLexRule(const char *tokenName, const char *regexString);
LexRule *getMatchingLexRule(LexRule **rules, const char *buffer);
void freeLexRule(LexRule *rule);

int main(void) {
    FILE *inputFile;
    inputFile = fopen("input.txt", "r");

    LexRule **rules = NULL;
    char buffer[BUFFER_SIZE] = {0};
    int bufferIndex = 0;


    arrput(rules, createLexRule("NUMBER", "[0-9][0-9]*"));
    arrput(rules, createLexRule("IF", "if"));
    arrput(rules, createLexRule("ID", "[a-zA-Z][a-zA-Z0-9_]*"));
    arrput(rules, createLexRule("WHITESPACE", "."));

    if (!inputFile) {
        fprintf(stderr, "input.txt not found");
        exit(1);
    }

    {
        int matchFlag = 0;
        char c = fgetc(inputFile);
        while (c != EOF) {
            int currentlyMatching;

            buffer[bufferIndex] = c;
            currentlyMatching = getMatchingLexRule(rules, buffer) != NULL;

            if (matchFlag && !currentlyMatching) {
                buffer[bufferIndex] = '\0';
                printf("%s -> %s\n", buffer, getMatchingLexRule(rules, buffer)->token);

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
        if (lastRule) printf("%s -> %s\n", buffer, lastRule->token);
    }

    {
        int i;
        for (i = 0; i < arrlen(rules); ++i) {
            freeLexRule(rules[i]);
        }
    }

    arrfree(rules);
    fclose(inputFile);

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
    rule->token = tokenName;
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
    free(rule);
}
