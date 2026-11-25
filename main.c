#include "./header/utils.h"
#include "./header/lexer.h"
#include "./header/parser.h"
#include "./header/executor.h"
#include "./header/functions.h"
#include "./header/hashmap.h"
#include "./header/lineedit.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void addToken(Token **head, Token *t) {
    if ((*head) != NULL) {
        (*head)->next = t;
    }
    (*head) = t;
}

void printTokenList(Token *curr) {
    while(curr) {
        printToken(curr);
        curr = curr->next;
    }
}

/* =================================================== MAIN ============================================== */

int main(void) {
    // load and store builtin functions
    Map *m = createMap();
    put(m, "greetings", greetings);

    printBox("Comi's terminal :)");
    while(1) {
        char *path = (char *)xmalloc(PATH_MAX);
        char *ptr = getwd(path);
        if (ptr == NULL) {
            fprintf(stderr, "Unable to get current working directory: %s\n", path);
            exit(1);
        }

        snprintf(ptr, PATH_MAX, "%s >> ", ptr);
        char *buf = readLine(ptr);
        if (buf == NULL || strcmp(buf, "exit") == 0) break;              /* Ctrl-D a riga vuota */
        if (buf[0] == '\0') { free(buf); continue; }

        addHistory(buf);

        // create lexer
        Lexer *l = createLexer(buf);
        Token *head = NULL;
        Token *first = NULL;
        Token *out = NULL;

        int lexerr = 0;

        do {
            out = (Token *)xmalloc(sizeof(Token));
            out->next = NULL;
            lexerr = next(l, &out);
            if (lexerr == 1) break; // errore, esci dal ciclo, libera memoria e ricomincia

            addToken(&head, out);

            if(!first) {
                first = head;
            }
            if (out->type == TYPE_EOF) {
                break;
            }
        } while(out != NULL && out->type != TYPE_EOF);

        if(lexerr == 0) {
            // if there are no syntactic errors, create a parser and parse the command
            Parser *p = createParser(first);
            PToken *op = parseList(p, 0);
            if (op == NULL) {
                continue;
            }
            
            execute(op, m, NULL, NULL);

            freeParser(p);
            freePToken(op);
        } else {
            freeTokenList(first);
        }

        freeLexer(l);
        free(ptr);
    }
    freeMap(m);
    return 0;
}