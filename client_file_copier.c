#define _POSIX_C_SOURCE 200809L //getline
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "client_file_copier.h"

#define MAX_LEN_FILE 2000

void file_copier_create(struct file_copier* self,\
                     char* _filename, struct client_socket* _socket){
    self->filename = _filename;
    self->socket = _socket;
}

bool file_copier_start(struct file_copier* self) {
    FILE* file = fopen(self->filename, "r");
    if (!file) {
	    return false;
    }

    //int i = 0;
    char* lineptr = NULL; size_t n = 0; size_t len;
    while (true) { //!feof(file) && i < MAX_LEN_FILE) {
        //int c = fgetc(file);
        //self->path[i] = c;
	    //i++;
        len = getline(&lineptr, &n, file);
        if (len == -1) break;
        client_socket_send_request(self->socket, len, lineptr);
    }
    //self->path[i-2] = (int)'\0'; //me estaba leyendo un \n y un eof o algo asi
    fclose(file);
    free(lineptr);
    //*self->path_len = i - 2;
    return true;
}

