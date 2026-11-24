#ifndef HASHMAP_CHECK
#define HASHMAP_CHECK

#define MAP_SIZE 16

typedef struct entry{
    char *key;
    int (*func)(char **argv);
    struct entry* next;
} Entry;

typedef struct map {
    Entry *e[MAP_SIZE];
} Map;

Map *createMap();

void freeMap(Map *m);

void put(Map *m, char *key, int(*func)(char **argv));

Entry *get(Map *m, const char *str);

void printMap(Map *m);

#endif