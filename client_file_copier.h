#ifndef CLIENT_FILE_COPIER_H
#define CLIENT_FILE_COPIER_H
#include <stdint.h>

struct file_copier {
    char* filename;
    char* path;
    size_t* path_len;
};

void file_copier_create(struct file_copier* self,\
                     char* filename, size_t* path_len, char* path);

/*
abre el archivo cuyo nombre tiene almacenado como atributo
y copia su contenido en un buffer también almacenado como atributo
*/
bool file_copier_start(struct file_copier* self);

#endif
