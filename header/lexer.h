#ifndef LEXER_CHECK
#define LEXER_CHECK

#include "token.h"

typedef struct lexer{
    const char *command;
    int index;
} Lexer;

Lexer *createLexer(const char *str);

void freeLexer(Lexer *l);

int next(Lexer *l, Token **out);

#endif