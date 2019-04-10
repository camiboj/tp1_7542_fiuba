#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "common_socket.h"

#define MAX_WAITING_CLIENTS 20

//+++++++ IGUAL ++++++++++

int socket_receive_some(struct socket *self, char* buf, \
                                size_t size) {
    return recv(self->current_peerskt, buf , size, MSG_NOSIGNAL);
}

int socket_send_all(struct socket *self, \
                                size_t size, char* buf) {
    int bytes_sent = 0;
    int s = 0;
    bool is_the_socket_valid = true;

    while (bytes_sent < size && is_the_socket_valid) {
        s = send(self->current_peerskt, &buf[bytes_sent], \
                size-bytes_sent, MSG_NOSIGNAL);
        if (s <= 0) {
            return -1;
        } else {
            bytes_sent += s;
        }
    }
    return bytes_sent;
}


void socket_create(struct socket *self, char* _host,\
                            char* _port) {
    self->host = _host;
    self->port = _port;
    self->skt = 0; 
    self->current_peerskt = 0;
}

void socket_destroy(struct socket *self) {
    freeaddrinfo(self->result);
    shutdown(self->skt, SHUT_RDWR);
    close(self->skt);
}

bool socket_start(struct socket *self) {
    int s = 0;

    struct addrinfo hints; 

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;   

    s = getaddrinfo(self->host, self->port, &hints, &self->result);

    if (s != 0) { 
        return false;
    }
    return true;
}



//+++++ UNICO ++++++++

bool socket_connect_with_clients(struct socket *self) {
    struct addrinfo *ptr = self->result;
    int s = 0;

    self->skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (self->skt == -1) {
        return false;
    }

    int val = 1;
    s = setsockopt(self->skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (s == -1) {
        return false;
    }    

    s = bind(self->skt, ptr->ai_addr, ptr->ai_addrlen);
    if (s == -1) {
        return false;
    }
    //
    s = listen(self->skt, MAX_WAITING_CLIENTS);
    if (s == -1) {
        return false;
    }
    return true;
}

void socket_disable_client(struct socket *self) {
        shutdown(self->current_peerskt, SHUT_RDWR); 
        close(self->current_peerskt);
}

//-1 si falla
int socket_accept_client(struct socket *self){
    int peerskt = accept(self->skt, NULL, NULL);
    self->current_peerskt = peerskt;
    return peerskt;
}


//+++++ UNICO ++++++++

bool socket_connect_with_server(struct socket *self) {
    int s = 0;
    bool are_we_connected = false;
    struct addrinfo *ptr;

    for (ptr = self->result; ptr != NULL && are_we_connected == false;\
        ptr = ptr->ai_next) {
        self->skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (self->skt == -1) continue;
        s = connect(self->skt, ptr->ai_addr, ptr->ai_addrlen);
        are_we_connected = (s != -1);
    }
    self->current_peerskt = self->skt;

    return are_we_connected;
}

void socket_disables_send_operations(struct socket *self) {
    shutdown(self->skt, SHUT_WR);
}
