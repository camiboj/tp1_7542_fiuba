
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "server_tamplate.h"

#define TO_REPLACE "{{datos}}"
#define SIZE_TO_REPLACE 9
#define MAX_LEN_REPLY 2000

bool server_template_create(struct server_template *self, char* filename) {
    self->text = malloc(MAX_LEN_REPLY);
    if ( !self->text ) return false;
    FILE* file = fopen(filename, "r");
    if (!file) {
        free(self->text);
	    return false;
    }
    
    size_t i = 0;
    while (!feof(file) && i < MAX_LEN_REPLY) {
        self->text[i] = (char) fgetc(file);
        i++;
    }
    self->text[i-1] = '\0';
    self->to_replace = strstr(self->text, TO_REPLACE);
    fclose(file);
    return true;
}

char* server_template_cat(struct server_template *self, char* replacement) {
    char* reply = malloc(MAX_LEN_REPLY);
    if ( !reply ) return NULL;
    char aux[MAX_LEN_REPLY];
    snprintf(aux, MAX_LEN_REPLY, "%s", self->text);
    
    char* to_replace = strstr(aux, TO_REPLACE);
  
    snprintf(to_replace, strlen(to_replace) - 1, "%s", replacement);
    snprintf(reply, MAX_LEN_REPLY - 1, "%s", aux);
    
    int len = strlen(&to_replace[SIZE_TO_REPLACE]);
    snprintf(&reply[strlen(reply)], len,"%s", &to_replace[SIZE_TO_REPLACE]);
    return reply;
}

void server_template_destoy(struct server_template *self) {
    free(self->text);
}
