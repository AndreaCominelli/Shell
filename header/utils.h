#ifndef UTILS_CHECK
#define UTILS_CHECK

#include <stdio.h>

void *xmalloc(size_t size);

void *xrealloc(void *ptr, size_t size);

char *xstrdup(const char *s);

void split(char **dst, const char *str, const char delimiter);

void printBox(const char *s);

#endif