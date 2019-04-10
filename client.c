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

#include "client_file_sender.h"

#define MAX_LEN_FILENAME 100
#define MAX_LEN_BUF 2000

bool resive(struct socket* socket) {
    int status = 0;
    int received = 0;
    char buf[MAX_LEN_BUF];

    while ( true ) {
        status = socket_receive_some(socket, &buf[received], \
                 MAX_LEN_BUF - received - 1);
        if (status < 0) { //socker error
            return false;
        } else if (status == 0) {
            break;
        } else {
            received = status; 
            buf[received] = 0;
            printf("%s", buf);
            received = 0; 
        }
    }
    printf("\n");
    return true;
}


int main(int argc, char* argv[]) {
    if (argc != 3 && argc !=4) {
	    fprintf(stderr, "Uso:\n./client <direccion> <puerto> [<input>]\n");
	    return 1;
    }
    char filename[MAX_LEN_FILENAME];
    if (argc == 3) {
	    char* status = fgets(filename, MAX_LEN_FILENAME, stdin);
	    filename[strlen(filename) -1] = '\0';
	    if (!status) {
            return 1;
        }
    } else {
	    snprintf(filename, MAX_LEN_FILENAME, "%s", argv[3]);
    }

    struct file_sender fs;
    char* host = argv[1];    
    char* port = argv[2];
    struct socket socket;
    socket_create(&socket, host, port);

    if (!socket_start(&socket)) {
        socket_destroy(&socket);
        return 1;
    }
    if (!socket_connect_with_server(&socket)) {
        socket_destroy(&socket);
        return 1;
    }

    int status = 0;
    file_sender_create(&fs, filename, &socket);

    if (!file_sender_start(&fs)){
        status = 1;
    }


    socket_disables_send_operations(&socket);
    if ( !resive(&socket) ) {
        status = 1;
    }

    socket_destroy(&socket);
    file_sender_destroy(&fs);
    return status;
}
