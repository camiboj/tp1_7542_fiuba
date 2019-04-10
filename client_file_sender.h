#ifndef CLIENT_FILE_SENDER_H
#define CLIENT_FILE_SENDER_H
#include <stdint.h>
#include "common_socket.h"

struct file_sender {
    char* filename;
    struct socket* socket;
};

void file_sender_create(struct file_sender* self,\
                     char* _filename, struct socket* _socket);

/*
abre el archivo cuyo nombre tiene almacenado como atributo
y envia su contenido a través de socket que tiene almacenado como atributo
*/
bool file_sender_start(struct file_sender* self);

void file_sender_destroy(struct file_sender* self);

#endif
