#ifndef CLIENT_FILE_COPIER_H
#define CLIENT_FILE_COPIER_H
#include <stdint.h>
#include "client_socket.h"

struct file_copier {
    char* filename;
    struct client_socket* socket;
};

void file_copier_create(struct file_copier* self,\
                     char* _filename, struct client_socket* _socket);

/*
abre el archivo cuyo nombre tiene almacenado como atributo
y copia su contenido en un buffer también almacenado como atributo
*/
bool file_copier_start(struct file_copier* self);

#endif
