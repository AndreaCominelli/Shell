#include "../header/utils.h"
#include "../header/hashmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned long hash(const char *str){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int computeKey(const char *str) {
    return hash(str) % MAP_SIZE;
}

/* ========================================= MEMORY HANDLING =============================================== */

Map *createMap() {
    Map *m = xmalloc(sizeof(Map));
    for(int i = 0; i < MAP_SIZE; i++) {
        m->e[i] = 0;
    }

    return m;
}

Entry *createEntry(char *key, int(*func)(char **argv)) {
    Entry *e = xmalloc(sizeof(Entry));
    e->next = NULL;
    e->func = func;
    e->key = key;

    return e;
}

void freeEntry(Entry* e) {
    Entry *curr = e;
    while(curr) {
        Entry *next = curr->next;
        free(curr);
        curr = next;
    }
}

void freeMap(Map *m) {
    for(int i = 0; i < MAP_SIZE; i++) {
        freeEntry(m->e[i]);
    }
    free(m);
}

void put(Map *m, char *key, int(*func)(char **argv)) {
    Entry *e = createEntry(key, func);
    int k = computeKey(key);
    if (m->e[k] == 0) {
        m->e[k] = e;
    } else {
        Entry *curr = m->e[k];
        Entry *prev = NULL;
        int override = 0;
        while(curr) {
            if(strcmp(curr->key, e->key) == 0) {
                e->next = curr->next;
                if (prev == NULL)
                    m->e[k] = e;
                else
                    prev->next = e;  
                override = 1;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        if (override == 0) {
            prev->next = e;
        }
    }
}

Entry *get(Map *m, const char *str) {
    int k = computeKey(str);
    Entry *curr = m->e[k];
    while(curr) {
        if(strcmp(curr->key, str) == 0) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

void printMap(Map *m) {
    for(int i = 0; i < MAP_SIZE; i++) {
        if(m->e[i]) {
            printf("%s, ", m->e[i]->key);
            Entry *next = m->e[i]->next;
            if(next) {
                while(next) {
                    printf("%s, ", next->key);
                    next = next->next;
                }
            }
            printf("\n");
        } else {
            printf("Value not set\n");
        }
    }
}