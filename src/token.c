#include <stdio.h>
#include "../header/token.h"

void printToken(Token *t) {
    switch(t->type) {
        case 0:
            printf("Operator: %s\n", t->val.str);
            break;
        case 1:
            printf("Word: %s\n", t->val.str);
            break;
        case 2:
            printf("EOF Token\n");
            break;
    }
}