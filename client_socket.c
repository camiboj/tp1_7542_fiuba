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
#include "client_socket.h"

#define REQUEST_MAX_LEN 2000
#define RESPONSE_MAX_LEN 2000
#define LEN_HOST 13
#define LEN_PORT 6


bool client_socket_send_request(struct client_socket *self) {
    int s = 0;
    int bytes_sent = 0;
    
    while (bytes_sent < self->request_len) {
        s = send(self->skt, &self->request[bytes_sent], \
                self->request_len - bytes_sent, MSG_NOSIGNAL);

        if (s <= 0) {
            shutdown(self->skt, SHUT_RDWR);
            close(self->skt);
            return false;
        } else {
            bytes_sent += s;
        }
    }
    shutdown(self->skt, SHUT_WR);
    return true;
}

bool client_socket_receive_reponse(struct client_socket *self) {
    int s = 0;
    int bytes_receive = 0;
    char response[RESPONSE_MAX_LEN];

    while ( true ) {
        s = recv(self->skt, &response[bytes_receive], \
                 RESPONSE_MAX_LEN - bytes_receive - 1, MSG_NOSIGNAL);
        if (s < 0) { //socker error
            return false;
        } else if (s == 0) {
            break;
        } else {
            bytes_receive = s; 
            response[bytes_receive] = 0;
            printf("%s", response);
            bytes_receive = 0; 
        }
    }
    printf("\n");
    return true;
}

bool client_socket_start(struct client_socket *self) {
    int s = 0;
    bool are_we_connected = false;

    struct addrinfo hints;
    struct addrinfo *result, *ptr;

    self->skt = 0;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;            

    s = getaddrinfo(self->host, self->port, &hints, &result);

    if (s != 0) { 
        return false;
    }

   
    for (ptr = result; ptr != NULL && are_we_connected == false;\
        ptr = ptr->ai_next) {
        self->skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (self->skt == -1) {
        } else {
            s = connect(self->skt, ptr->ai_addr, ptr->ai_addrlen);
            if (s == -1) {
                close(self->skt);
            }
            are_we_connected = (s != -1);
        }
    }

   freeaddrinfo(result);

    return are_we_connected;
}

void client_socket_create(struct client_socket *self, size_t _request_len,\
                         char* _request, char* _host, char* _port) {
    self->host = malloc(LEN_HOST);
    self->port = malloc(LEN_PORT);
    self->request = malloc(_request_len + 1);
    
    self->request_len = _request_len;
    snprintf(self->request, _request_len  + 1, "%s", _request);
    snprintf(self->host, LEN_HOST , "%s", _host);
    snprintf(self->port, LEN_PORT , "%s", _port);
}

void client_socket_destroy(struct client_socket *self) {
    shutdown(self->skt, SHUT_RDWR);
    close(self->skt);

    free(self->request);
    free(self->host);
    free(self->port);
}
