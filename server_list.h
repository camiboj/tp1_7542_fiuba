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

/*
Dada una lista y una clave se agrega la clave a la lista 
y se inicializa su valor.
En caso de ya existir la clave solamente se aumenta en uno su valor.
*/
bool list_insert(struct List *self, char* _key);

/*
Imprime todos los datos almacenados de la forma
* <clave1>: <valor1>
* <clave2>: <valor2>
*/
void list_print(struct List *self);

#endif

