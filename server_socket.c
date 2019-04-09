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
#include "server_socket.h"

#define LEN_PORT 6
#define MAX_WAITING_CLIENTS 20
#define MAX_LEN_BUF 2000 


bool server_socket_create(struct server_socket *self, char* _port) {
    self->port = malloc(LEN_PORT);
    if ( !self->port ) return false;
    snprintf(self->port, LEN_PORT , "%s", _port);
    self->current_peerskt = 0;
    return true;
}

bool server_socket_start(struct server_socket *self) {
    int s = 0;

    struct addrinfo hints; 
    struct addrinfo *ptr;

    int skt = 0; 

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;     

    s = getaddrinfo(NULL, self->port, &hints, &ptr);

    if (s != 0) { 
        return false;
    }

    skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (skt == -1) {
        freeaddrinfo(ptr);
        return false;
    }

    int val = 1;
    s = setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (s == -1) {
        close(skt);
        freeaddrinfo(ptr);
        return false;
    }    

    s = bind(skt, ptr->ai_addr, ptr->ai_addrlen);
    if (s == -1) {
        close(skt);
        freeaddrinfo(ptr);
        return false;
    }

    freeaddrinfo(ptr);
    s = listen(skt, MAX_WAITING_CLIENTS);
    if (s == -1) {
        close(skt);
        return false;
    }
    self->skt = skt;
    return true;
}

//-1 si falla
int server_socket_accept_client(struct server_socket *self){
    int peerskt = accept(self->skt, NULL, NULL);
    self->current_peerskt = peerskt;
    return peerskt;
}

void server_socket_destroy(struct server_socket *self) {
    free(self->port);
    shutdown(self->skt, SHUT_RDWR);
    close(self->skt);
}

char* server_socket_receive_message(struct server_socket *self) { 
    char* buf = malloc(MAX_LEN_BUF);
    if ( !buf ) return NULL;
    memset(buf, 0, MAX_LEN_BUF);
    int received = 0;
    int s = 0;
    bool is_the_socket_valid = true;
    while (received < MAX_LEN_BUF && is_the_socket_valid) {
        s = recv(self->current_peerskt, buf + received, \
                    MAX_LEN_BUF - received, MSG_NOSIGNAL);
        if (s == 0) { 
            is_the_socket_valid = false;
        } else if (s < 0) { 
            is_the_socket_valid = false;
        } else {
            received += s;
        }
    }
    return buf;
}

int server_socket_send_message(struct server_socket *self, char* buf, \
                                int size) {
    int sent = 0;
    int s = 0;
    bool is_the_socket_valid = true;

    while (sent < size && is_the_socket_valid) {
        s = send(self->current_peerskt, &buf[sent], size-sent, MSG_NOSIGNAL);
        if (s <= 0) {
            return -1;
        } else {
            sent += s;
        }
    }
    return sent;
}

void server_socket_disable_client(struct server_socket *self) {
        shutdown(self->current_peerskt, SHUT_RDWR); 
        close(self->current_peerskt);
}
