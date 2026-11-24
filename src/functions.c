#include <stdio.h>
#include <string.h>
#include "../header/functions.h"

int countargs(char **argv) {
    int i = 0;
    while(argv[i]) {
        i++;
    }
    return i;
}

// argv contains the entire program argument list with the program name in the first position
int greetings(char **argv) {
    if (argv != NULL && countargs(argv) == 2) {
        const char *name = argv[1];
        fprintf(stdout, "Hello %s\n", name);
        return 0;
    }
    fprintf(stderr, "Too many arguments\n");
    return 1;
}