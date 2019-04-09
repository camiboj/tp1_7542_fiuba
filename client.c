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

#include "client_file_copier.h"

#define MAX_LEN_FILENAME 100
#define MAX_LEN_FILE 2000

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

    struct file_copier copier;
    //char path[MAX_LEN_FILE];
    //size_t path_len;

    char* host = argv[1];    
    char* port = argv[2];
    struct client_socket socket;
    client_socket_create(&socket, host, port);
    if (!client_socket_start(&socket)) {
        return 1;
    }
    file_copier_create(&copier, filename, &socket);

    if (!file_copier_start(&copier)){
	    return 1;
    }


    client_socket_disables_send_operations(&socket);
    //client_socket_send_request(&socket);
    client_socket_receive_reponse(&socket);
    client_socket_destroy(&socket);
    return 0;
}
