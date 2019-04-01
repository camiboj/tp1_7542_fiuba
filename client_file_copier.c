#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "client_file_copier.h"

#define MAX_LEN_FILE 2000

void file_copier_create(struct file_copier* self,\
                     char* filename, size_t* path_len, char* path){
    self->filename = filename;
    self->path = path;
    self->path_len = path_len;
}

bool file_copier_start(struct file_copier* self) {
    FILE* file = fopen(self->filename, "r");
    if (!file) {
	    return false;
    }

    int i = 0;
    while (!feof(file) && i < MAX_LEN_FILE) {
	    int c = fgetc(file);
        self->path[i] = c;
	    i++;
    }
    self->path[i-2] = (int)'\0'; //me estaba leyendo un \n y un eof o algo asi
    fclose(file);
    *self->path_len = i - 2;
    return true;
}

