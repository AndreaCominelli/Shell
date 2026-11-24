#include "../header/lexer.h"
#include "../header/utils.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define SYMBOLS "|&;<>"

Lexer *createLexer(const char *str) {
    Lexer *l = (Lexer *)xmalloc(sizeof(Lexer));
    l->command = str;
    l->index = 0;

    return l;
}

void freeLexer(Lexer *l) {
    if(l != NULL) {
        free((char *)l->command);
        free(l);
    }
}

int next(Lexer *l, Token **out) {
    if (l->command[l->index] == 0) {
        /* imposta (*out) a un token EOF e return */
        (*out)->type = 2;
        (*out)->val.str = NULL;
        (*out)->val.len = 0;
        return 0;
    }

    while(l->command[l->index] != 0) {
        int start = l->index;

        if (l->command[l->index] == '|') {
            l->index++;
            if(l->command[l->index] == '|') {
                l->index++;
            } 
            if(l->command[l->index] != 0 && !isspace(l->command[l->index])){
                fprintf(stderr, "Error at %d: unrecognised character '%c'\n", l->index, l->command[l->index]);
                return 1;
            }
            size_t len = l->index - start;
            char *buf = (char *)xmalloc(sizeof(char) * len + 1);
            memcpy(buf, &l->command[start], len);
            buf[len] = 0;

            (*out)->type = 0;
            (*out)->val.str = buf;
            (*out)->val.len = len;

            if(l->command[l->index]) {
                while(isspace(l->command[l->index])) {
                    l->index++;
                }
            }

            return 0;
        }

        if (l->command[l->index] == '&') {
            l->index++;
            if(l->command[l->index] == '&') {
                l->index++;
            }
            if(l->command[l->index] != 0 && !isspace(l->command[l->index])){
                fprintf(stderr, "Error at %d: unrecognised character '%c'\n", l->index, l->command[l->index]);
                return 1;
            }
            size_t len = l->index - start;
            char *buf = (char *)xmalloc(sizeof(char) * len + 1);
            memcpy(buf, &l->command[start], len);
            buf[len] = 0;

            (*out)->type = 0;
            (*out)->val.str = buf;
            (*out)->val.len = len;

            if(l->command[l->index]) {
                while(isspace(l->command[l->index])) {
                    l->index++;
                }
            }

            return 0;
        }

        if (l->command[l->index] == '>') {
            l->index++;
            if(l->command[l->index] == '>') {
                l->index++;
            }
            if(l->command[l->index] != 0 && !isspace(l->command[l->index])){
                fprintf(stderr, "Error at %d: unrecognised character '%c'\n", l->index, l->command[l->index]);
                return 1;
            }
            size_t len = l->index - start;
            char *buf = (char *)xmalloc(sizeof(char) * len + 1);
            memcpy(buf, &l->command[start], len);
            buf[len] = 0;

            (*out)->type = 0;
            (*out)->val.str = buf;
            (*out)->val.len = len;

            if(l->command[l->index]) {
                while(isspace(l->command[l->index])) {
                    l->index++;
                }
            }

            return 0;
        }

        if (l->command[l->index] == '<') {
            l->index++;
            if(l->command[l->index] != 0 && !isspace(l->command[l->index])){
                fprintf(stderr, "Error at %d: unrecognised character '%c'\n", l->index, l->command[l->index]);
                return 1;
            }
            size_t len = l->index - start;
            char *buf = (char *)xmalloc(sizeof(char) * len + 1);
            memcpy(buf, &l->command[start], len);
            buf[len] = 0;

            (*out)->type = 0;
            (*out)->val.str = buf;
            (*out)->val.len = len;

            if(l->command[l->index]) {
                while(isspace(l->command[l->index])) {
                    l->index++;
                }
            }

            return 0;
        }

        if(l->command[l->index] == ';') {
            l->index++;
            if(l->command[l->index] != 0 && !isspace(l->command[l->index])){
                fprintf(stderr, "Error at %d: unrecognised character '%c'\n", l->index, l->command[l->index]);
                return 1;
            }
            size_t len = l->index - start;
            char *buf = (char *)xmalloc(sizeof(char) * len + 1);
            memcpy(buf, &l->command[start], len);
            buf[len] = 0;

            (*out)->type = 0;
            (*out)->val.str = buf;
            (*out)->val.len = len;

            if(l->command[l->index]) {
                while(isspace(l->command[l->index])) {
                    l->index++;
                }
            }

            return 0;
        }

        // word
        if(!strchr(SYMBOLS, l->command[l->index]) ||
            l->command[l->index] == '\'' ||
            l->command[l->index] == '"'
        ) {
            if(l->command[l->index] == '\'') {
                l->index++;
                while(l->command[l->index] != '\'') {
                    if (l->command[l->index] == '"') {
                        fprintf(stderr, "Error at %d: found double quote in single quotes.\n", l->index);
                        return 1;
                    }
                    l->index++;
                }
                l->index++;
            } else if(l->command[l->index] == '"') {
                l->index++;
                while(l->command[l->index] != '"') {
                    if (l->command[l->index] == '\'') {
                        fprintf(stderr, "Error at %d: found single quote in double quotes.\n", l->index);
                        return 1;
                    }
                    l->index++;
                }
                l->index++;
            } else {
                l->index++;
                while (!isspace(l->command[l->index]) &&
                   l->command[l->index] != '|' &&
                   l->command[l->index] != '&' &&
                   l->command[l->index] != ';' &&
                   l->command[l->index] != '(' &&
                   l->command[l->index] != ')' &&
                   l->command[l->index] != '<' &&
                   l->command[l->index] != '>' &&
                   l->command[l->index] != 0)
                {
                    l->index++;
                }
            }

            size_t len = l->index - start;
            char *buf = (char *)xmalloc(sizeof(char) * len + 1);
            memcpy(buf, &l->command[start], len);
            buf[len] = 0;

            (*out)->type = 1;
            (*out)->val.str = buf;
            (*out)->val.len = len;

            if(l->command[l->index]) {
                while(isspace(l->command[l->index])) {
                    l->index++;
                }
            }

            return 0;
        }
    }
    return 1;
}