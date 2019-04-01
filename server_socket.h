#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <stdlib.h>


struct server_socket {
    char* port;
    int skt;
    int current_peerskt;
};


void server_socket_create(struct server_socket *self, char* _port);
void server_socket_destroy(struct server_socket *self);
bool server_socket_start(struct server_socket *self);
int server_socket_accept_client(struct server_socket *self);
char* server_socket_receive_message(struct server_socket *self);
int server_socket_send_message(struct server_socket *self, char* buf, int size);
void server_socket_disable_client(struct server_socket *self);

#endif
