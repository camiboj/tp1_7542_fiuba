#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server_list.h"

struct nodo {
    char* key;
    long int value;
    struct nodo* next;
} typedef nodo_t;



bool nodo_create(nodo_t* self, char* _key) {
    self->value = 1;
    self->next = NULL;
    size_t len_key = strlen(_key) + 1;
    self->key = malloc(len_key);
    if ( !self->key ) return false;
    snprintf(self->key, len_key, "%s", _key);
    return true;
}

void list_create(struct List *self) {
    self->first = NULL;
    self->last = NULL;
    //self->len = 0;
}

void list_destroy(struct List *self) {
    nodo_t* current = self->first;
    nodo_t* aux; 
    while (current) {
	aux = current->next;
        free(current->key);
	free(current);
	current = aux;
    }
}

bool list_insert(struct List *self, char* _key) {
    bool status = true;
    nodo_t* current = self->first;
    while (current) {
	    if ( !strcmp(current->key, _key) ){
	        current->value++;
	        return true;
	    }
	    current = current->next;
    }
    
    nodo_t* nodo = malloc(sizeof(nodo_t));
    if (!nodo) return false;
    status = nodo_create(nodo, _key);
    if ( !status ) {
        free(nodo);
        return false;
    }
    if (!self->first) {
	    self->first = nodo;
	    self->last = nodo;
	    return true;
    }
    self->last->next = nodo;
    self->last = nodo;
    return true;
}

/*
void list_delete_first(struct List *self) {
    if (self->first == self->last) self->last = NULL;
    nodo_t* aux = self->first->next;
    free(self->first->key);
    free(self->first);
    self->first = aux;
}
*/

void list_print(struct List *self){
    printf("# Estadisticas de visitantes\n");
    nodo_t* current = self->first;
    while (current) {        
        printf("\n* %s: %ld", current->key, current->value);
        current = current->next;
    }
    printf("\n");
}
