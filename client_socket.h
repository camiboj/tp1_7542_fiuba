#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H
#include <stdlib.h>

struct client_socket {
    size_t request_len;
    char* request;
    char* host;
    char* port;
    int skt;
};

/*
Guarda los parametros que se necesitaran al inciar el socket
*/
void client_socket_create(struct client_socket *self, size_t _request_len,\
                         char* _request, char* _host, char* _port);
void client_socket_destroy(struct client_socket *self);
/*
Crea el socket definiendo la familia, el tipo de socket y el protocolo 
para poder conectarse al servidor por medio del port y host indicados.
*/
bool client_socket_start(struct client_socket *self);
bool client_socket_receive_reponse(struct client_socket *self);
bool client_socket_send_request(struct client_socket *self);

#endif
