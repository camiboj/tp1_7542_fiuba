
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "server_tamplate.h"


#define TO_REPLACE "{{datos}}"
#define SIZE_TO_REPLACE 9
#define MAX_LEN_REPLY 2000




bool template_create(struct template *self, char* filename, \
                    struct socket* skt) {
    self->skt = skt;
    FILE* file = fopen(filename, "r");
    if (!file) {
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

void template_send_cat(struct template *self, char* replacement) {
    char* p = strstr(self->text, TO_REPLACE);
    socket_send_all(self->skt, p - self->text, self->text);
    socket_send_all(self->skt, strlen(replacement), replacement);
    socket_send_all(self->skt, strlen(p + SIZE_TO_REPLACE) - 1 , \
                    p + SIZE_TO_REPLACE);
}

void template_destroy(struct template *self) {
    //do nothing
}
