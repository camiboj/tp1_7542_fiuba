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


void server_socket_create(struct server_socket *self, char* _port) {
    self->port = malloc(LEN_PORT);
    snprintf(self->port, LEN_PORT , "%s", _port);
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
        //printf("Error in getaddrinfo: %s\n", gai_strerror(s));
        return false;
    }

    skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (skt == -1) {
        //printf("Error: %s\n", strerror(errno));
        freeaddrinfo(ptr);
        return false;
    }

    int val = 1;
    s = setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (s == -1) {
        //printf("Error: %s\n", strerror(errno));
        close(skt);
        freeaddrinfo(ptr);
        return false;
    }    

    s = bind(skt, ptr->ai_addr, ptr->ai_addrlen);
    if (s == -1) {
        //printf("Error: %s\n", strerror(errno));
        close(skt);
        freeaddrinfo(ptr);
        return false;
    }

    freeaddrinfo(ptr);
    s = listen(skt, MAX_WAITING_CLIENTS);
    if (s == -1) {
        //printf("Error: %s\n", strerror(errno));
        close(skt);
        return false;
    }
    self->skt = skt;
    return true;
}

//-1 si falla
int server_socket_accept_client(struct server_socket *self){
    int peerskt = accept(self->skt, NULL, NULL);
    return peerskt;
}

void server_socket_destroy(struct server_socket *self) {
    free(self->port);
    shutdown(self->skt, SHUT_RDWR);
    close(self->skt);
}
