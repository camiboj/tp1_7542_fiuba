#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <stdlib.h>
#include <stdbool.h>

struct server_socket {
    char* port;
    int skt;
    int current_peerskt;
};


/*
Crea e incializa el socket definiendo la familia, el tipo de socket y el 
protocolo para poder conectarse al cliente por medio del port y host indicados
*/
bool server_socket_create(struct server_socket *self, char* _port);

/*
Almacena los parametros necesarios para la incialización del socket.
*/
bool server_socket_start(struct server_socket *self);


void server_socket_destroy(struct server_socket *self);
int server_socket_accept_client(struct server_socket *self);

char* server_socket_receive_message(struct server_socket *self);
int server_socket_send_message(struct server_socket *self, char* buf, int size);

/*
Desactiva las operaciones de envío y recepción para el cliente y para si mismo
*/
void server_socket_disable_client(struct server_socket *self);

#endif
