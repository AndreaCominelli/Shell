#ifndef TOKEN_CHECK
#define TOKEN_CHECK
#include <stdio.h>

#define TYPE_OPERATOR   0
#define TYPE_WORD       1
#define TYPE_EOF        2

typedef struct token{
    int type;
    struct {
        char *str;
        size_t len;
    } val;
    struct token *next;
} Token;

void printToken(Token *t);

#endif