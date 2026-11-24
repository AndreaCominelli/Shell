#ifndef PARSER_CHECK
#define PARSER_CHECK

#include "token.h"

#define PTYPE_COMMAND   0
#define PTYPE_OPERATOR  1

#define REDIR_IN        0
#define REDIR_OUT       1
#define REDIR_APPEND    2

typedef struct ptoken {
    unsigned int type:4; // 4 bits for token type
    union {
        struct {
            char **argv; // the list of args for the program
            struct redir *red; // the list of redirection
        } cmd;
        struct {
            const char *name;
            struct ptoken *left;
            struct ptoken *right;
        } op;
    };
} PToken;

typedef struct redir {
    int type;            /* IN, OUT, APPEND */
    int fd;              /* fd da ridirigere (0, 1, 2...) */
    char *target;
    struct redir *next;
} Redir;

typedef struct parser {
    Token *list;
    int index;
} Parser;

Redir *createRedir(int type);

PToken *createPToken(int type);

PToken *createOPToken(const char *name);

PToken *createCMDToken(size_t size);

Parser *createParser(Token *l);

void freeParser(Parser *p);

void freeTokenList(Token *curr);

void freePToken (PToken *t);

void advance(Parser *p);

Token *peek(Parser *p);

void reset(Parser *p, int start);

void parseRedirection(Parser *p, PToken **out);

PToken *parseCommand(Parser *p, int start);

PToken *parsePipeline(Parser *p, int start);

PToken *parseList(Parser *p, int start);

void printArgv(PToken *t);

#endif