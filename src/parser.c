#include "../header/parser.h"
#include "../header/utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

PToken *createPToken(int type) {
    PToken *p = (PToken *)xmalloc(sizeof(PToken));
    p->type = type;

    return p;
}

PToken *createOPToken(const char *name) {
    PToken *op = createPToken(PTYPE_OPERATOR);
    op->op.left = NULL;
    op->op.right = NULL;
    op->op.name = name;

    return op;
}

void freeRedir(Redir *r) {
    if(r != NULL) {
        free(r->target);
        free(r);
    }
}

void freeRedirList(Redir *curr) {
    while(curr) {
        Redir *next = curr->next;
        freeRedir(curr);
        curr = next;
    }
}

void freeCMDToken (PToken *t) {
    if(t != NULL) {
        free(t->cmd.argv);
        freeRedirList(t->cmd.red);
    }
}

void freePToken (PToken *t) {
    if(t != NULL) {
        if(t->type == PTYPE_COMMAND) {
            freeCMDToken(t);
        }
        if(t->type == PTYPE_OPERATOR) {
            if(t->op.left) {
                if(t->op.left->type == PTYPE_OPERATOR) {
                    freePToken(t->op.left);
                } else if(t->op.left->type == PTYPE_COMMAND) {
                    freeCMDToken(t->op.left);
                }
            }
            if(t->op.right) {
                if(t->op.right->type == PTYPE_OPERATOR) {
                    freePToken(t->op.right);
                } else if(t->op.right->type == PTYPE_COMMAND) {
                    freeCMDToken(t->op.right);
                }
            }
        }
        free(t);
    }
}

PToken *createCMDToken(size_t size) {
    PToken *cmd = createPToken(PTYPE_COMMAND);
    cmd->cmd.argv = (char **)xmalloc(sizeof(char *) * (size + 1));
    cmd->cmd.argv[size] = 0;
    cmd->cmd.red = NULL;

    return cmd;
}


Redir *createRedir(int type) {
    Redir *r = (Redir *)xmalloc(sizeof(Redir));
    r->type = type;
    r->next = NULL;
    r->target = NULL;
    r->fd = -1;

    return r;
}

void printArgv(PToken *t) {
    switch(t->type) {
        case PTYPE_COMMAND:
            printf("argv: [");
            int index = 0;
            while(t->cmd.argv[index]) {
                printf("%s ", t->cmd.argv[index]);
                index++;
            }
            printf("]\n");
            if (t->cmd.red) {
                Redir *r = t->cmd.red;
                printf("redirections: ");
                while(r) {
                    printf("%d %s, ", r->type, r->target);
                    r = r->next; 
                }
                printf("\n");
            }
            break;
        case PTYPE_OPERATOR:
            printf("Operator: %s\n", t->op.name);
            printf("Left: ");
            printArgv(t->op.left);
            printf("Right: ");
            printArgv(t->op.right);
            break;
        default:
            break;
    }
}

Parser *createParser(Token *l)
{
    Parser *p = (Parser *)xmalloc(sizeof(Parser));
    p->list = l;
    p->index = 0;

    return p;
}

// the input token list made from the lexer
void freeTokenList(Token *curr) {
    while(curr) {
        Token *next = curr->next;
        free(curr);
        curr = next;
    }
}

void freeParser(Parser *p) {
    if(p != NULL) {
        freeTokenList(p->list);
        free(p);
    }
}

void advance(Parser *p)
{
    if (p)
    {
        p->index++;
    }
}

Token *peek(Parser *p)
{
    if (p)
    {
        int i = p->index;
        Token *curr = p->list;
        while (i)
        {
            curr = curr->next;
            i--;
        }

        return curr;
    }

    return NULL;
}

void reset(Parser *p, int start) {
    if(p) {
        p->index = start;
    }
}

void addRedirToToken(PToken *t, Redir *r) {
    if(t == NULL || r == NULL) exit(1);

    if(t->type == PTYPE_COMMAND) {
        Redir *curr = t->cmd.red;
        Redir *prev = curr;
        if (curr == NULL) {
            t->cmd.red = r;
        } else {
            while(curr->next) {
                prev = curr;
                curr = curr->next;
            }
            prev->next = r;
        }
    }
}

void parseRedirection(Parser *p, PToken **out)
{
    Token *op = peek(p);
    
    Redir *r = NULL;
    if (strcmp(op->val.str, ">>") == 0) {
        r = createRedir(REDIR_APPEND);
    } else if(strcmp(op->val.str, "<") == 0) {
        r = createRedir(REDIR_IN);
    } else if(strcmp(op->val.str, ">") == 0) {
        r = createRedir(REDIR_OUT);
    } else {
        fprintf(stderr, "Error at token %d: unrecognised redirection operator '%s'.\n", p->index, op->val.str);
        exit(1);
    }

    advance(p);
    Token *word = peek(p);
    if (word->type != TYPE_WORD){
        fprintf(stderr, "Error at token %d: '%s' is not a valid word.\n", p->index, word->val.str);
        exit(1);
    }

    r->target = xstrdup(word->val.str); // TODO: allocate memory, need to clean in free parser
    // r->target[word->val.len] = 0;

    addRedirToToken(*out, r);
}

PToken *parseCommand(Parser *p, int start)
{
    if (p == NULL || start < 0) {
        exit(1); // 
    }
    // count how many argv in the command
    size_t size = 0;
    while (peek(p)->type == TYPE_WORD) {
        size++;
        advance(p);
    }

    reset(p, start); // reset index to 0
    if(size == 0) {
        fprintf(stderr, "Error at token %d: '%s' is not a valid word.\n", p->index, peek(p)->val.str);
        exit(1);
    }

    int index = 0;
    PToken *ptoken = createCMDToken(size);
    ptoken->cmd.argv[index] = xstrdup(peek(p)->val.str);
    index++;
    
    advance(p);

    while (peek(p)->type == TYPE_WORD)
    {
        // copio in argv tutti i token word successivi (sono gli argomenti del programma)
        ptoken->cmd.argv[index] = xstrdup(peek(p)->val.str);
        advance(p);
        index++;
    }

    while(peek(p)->type == TYPE_OPERATOR && (
        strcmp(peek(p)->val.str, ">>") == 0 ||
        strcmp(peek(p)->val.str, ">") == 0 ||
        strcmp(peek(p)->val.str, "<") == 0
    )){
        parseRedirection(p, &ptoken);
        advance(p);
    };

    return ptoken;
}

PToken *parsePipeline(Parser *p, int start) {
    PToken *left = parseCommand(p, start);
    while(peek(p)->type == TYPE_OPERATOR && strcmp(peek(p)->val.str, "|") == 0) {
        advance(p);
        PToken *right = parseCommand(p, p->index);
        PToken *op = createOPToken("|");

        op->op.left = left;
        op->op.right = right;

        left = op;
    }
    return left;
}

PToken *parseList(Parser *p, int start) {
    PToken *left = parsePipeline(p, start);
     while(
        peek(p)->type == TYPE_OPERATOR &&
        (strcmp(peek(p)->val.str, ";") == 0 ||
        strcmp(peek(p)->val.str, "&&") == 0  ||
        strcmp(peek(p)->val.str, "||") == 0  ||
        strcmp(peek(p)->val.str, "&") == 0
    )) {
        char *symbol = peek(p)->val.str;
        advance(p);

        PToken *right = parsePipeline(p, p->index);
        PToken *op = createOPToken(symbol);

        op->op.left = left;
        op->op.right = right;

        left = op;
     }

     return left;
}
