#include "../header/utils.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if(ptr == NULL) {
        fprintf(stderr, "Unable to allocate memory: %s\n", strerror(errno));
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if(ptr == NULL) {
        fprintf(stderr, "Unable to reallocate memory: %s\n", strerror(errno));
        exit(1);
    }
    return ptr;
}

char *xstrdup(const char *s) {
    char *ptr = strdup(s);
    if(ptr == NULL) {
        fprintf(stderr, "Error %s: Unable to duplicate string '%s'\n", strerror(errno), s);
        exit(1);
    }

    return ptr;
}

int countChar(const char *str, const char c) {
    if(str == NULL) return 0;

    size_t l = strlen(str);
    int count = 0;

    for (int i = 0; i < l; i++) {
        if(str[i] == c) {
            count++;
        }
    }

    return count;
}

static int terminal_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        return w.ws_col;
    return 80; /* fallback se non è un terminale (pipe, file, ...) */
}

void printBox(const char *s) {
    int len = (int)strlen(s);
    int box_w = len + 4;                 /* '*' + spazio + testo + spazio + '*' */
    int pad = (terminal_width() - box_w) / 2;
    if (pad < 0) pad = 0;

    /* bordo superiore */
    printf("%*s", pad, "");
    for (int i = 0; i < box_w; i++) putchar('*');
    putchar('\n');

    /* riga col testo */
    printf("%*s* %s *\n", pad, "", s);

    /* bordo inferiore */
    printf("%*s", pad, "");
    for (int i = 0; i < box_w; i++) putchar('*');
    putchar('\n');
}

/**
 * Split a sting in n substrings at each delimiter char
 * and writes the result in dst.
 * The input str string must be null terminated.
 * If dst is null, then returns null
 */
void split(char **dst, const char *str, const char delimiter) {
    if (dst == NULL || str == NULL) exit(1);

    int numTokens = countChar(str, delimiter) + 1;
    const char *start = str;

    for (int i = 0; i < numTokens; i++) {
        const char *end = start;
        while (*end != delimiter && *end != '\0')
            end++;

        size_t len = end - start;
        dst[i] = malloc(len + 1);
        if (dst[i] == NULL) {
            while (i-- > 0) free(dst[i]);   /* rollback */
            exit(1);
        }
        memcpy(dst[i], start, len);
        dst[i][len] = 0;

        start = end + 1;                    /* salta il delimitatore */
    }

    // dst[numTokens] = 0;
}