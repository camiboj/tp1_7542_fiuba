#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <stdlib.h>


struct server_socket {
    char* port;
    int skt;
};


void server_socket_create(struct server_socket *self, char* _port);
void server_socket_destroy(struct server_socket *self);
bool server_socket_start(struct server_socket *self);
int server_socket_accept_client(struct server_socket *self);

#endif
