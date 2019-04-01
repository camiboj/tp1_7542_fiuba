#ifndef SERVER_LIST_H
#define SERVER_LIST_H
#include <stdbool.h>

struct nodo;

struct List {
    struct nodo* first;
    struct nodo* last;
    size_t len;
};

struct List;
void list_create(struct List *self);
void list_destroy(struct List *self);
bool list_insert(struct List *self, char* _key);
void list_print(struct List *self);

#endif

