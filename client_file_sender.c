#define _POSIX_C_SOURCE 200809L //getline
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "client_file_sender.h"

void file_sender_create(struct file_sender* self,\
                     char* _filename, struct socket* _socket){
    self->filename = _filename;
    self->socket = _socket;
}

bool file_sender_start(struct file_sender* self) {
    FILE* file = fopen(self->filename, "r");
    if (!file) {
	    return false;
    }
    
    char* lineptr = NULL; size_t n = 0; size_t len;
    while (true) { 
        len = getline(&lineptr, &n, file);
        if (len == -1) break;
        if ( socket_send_all(self->socket, len, lineptr) == -1 ) {
            return false;
        }
    }
    
    fclose(file);
    free(lineptr);
    return true;
}

void file_sender_destroy(struct file_sender* self) {
    //do nothing
}

