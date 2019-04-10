#ifndef COMMON_SOCKET_H
#define COMMON_SOCKET_H
#include <stdlib.h>
#include <stdbool.h>

struct socket {
    char* host;
    char* port;
    int skt;
    int current_peerskt;
    struct addrinfo *result;
};


/*
Crea e incializa el socket definiendo la familia, el tipo de socket y el 
protocolo para poder conectarse al cliente por medio del port y host indicados
*/
void socket_create(struct socket *self, char* _host, char* _port);

/*
Almacena los parametros necesarios para la incialización del socket.
*/
bool socket_start(struct socket *self);

bool socket_connect_with_clients(struct socket *self);

void socket_destroy(struct socket *self);
int socket_accept_client(struct socket *self);

int socket_receive_some(struct socket *self, char* buf, \
                                size_t size);



int socket_send_all(struct socket *self, \
                                size_t request_len, char*request);
/*
Desactiva las operaciones de envío y recepción para el cliente y para si mismo
*/
void socket_disable_client(struct socket *self);

//desabilita el canal de escritura
void socket_disables_send_operations(struct socket *self);
bool socket_connect_with_server(struct socket *self);

#endif
