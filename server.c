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

#include <arpa/inet.h>


#include "server_list.h"
#include "server_socket.h"
#include "server_sensor.h"
#include "server_request_processor.h"


#define TO_REPLACE "{{datos}}"
#define SIZE_TO_REPLACE 9
#define MAX_LEN_REPLY 2000




void copy_and_complet(char* tamplate_filename, char* reply, \
                      size_t len_reply, char* replacement) {
    char aux[MAX_LEN_REPLY];
    
    FILE* tamplate = fopen(tamplate_filename, "r");
    if (!tamplate_filename) {
	    return;
    }
    
    size_t i = 0;
    while (!feof(tamplate) && i < MAX_LEN_REPLY) {
        aux[i] = (char) fgetc(tamplate);
        i++;
    }
    aux[i-1] = '\0';

    char* to_replace = strstr(aux, TO_REPLACE);
  
    snprintf(to_replace, strlen(to_replace) - 1, "%s", replacement);
    snprintf(reply, len_reply - 1, "%s", aux);
    
    int len = strlen(&to_replace[SIZE_TO_REPLACE]);
    snprintf(&reply[strlen(reply)], len,"%s", &to_replace[SIZE_TO_REPLACE]);
    fclose(tamplate);
}



bool process_client(struct List* list, char* temperature, \
                    char* tamplate_filename, struct server_socket* socket) {
    char* buffer = server_socket_receive_message(socket);

    struct server_req_proc processor;
    req_proc_create(&processor, buffer);

    char* answer = req_porc_method_resource(&processor);
    server_socket_send_message(socket, answer, strlen(answer));

    if (! req_porc_is_method_resource_valid(&processor)) {
        req_proc_destroy(&processor);
        free(buffer);
        free(answer);
        return false;
    }
    char* us_ag = req_porc_user_agent(&processor);
    if (!us_ag) return false;
    list_insert(list, us_ag);
    
    //template
    char reply[MAX_LEN_REPLY];
    copy_and_complet(tamplate_filename, reply, MAX_LEN_REPLY, temperature);
    
    server_socket_send_message(socket, reply, strlen(reply));
    
    req_proc_destroy(&processor);
    free(buffer);
    free(answer);
    free(us_ag);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc !=4) {
	    fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
	    return 1;
    }
    
    char* port = argv[1];
    char* sensor_filename = argv[2];
    char* template_filename = argv[3];
    
    // (port, sensor_filename, template_filename);
    //int (char* port, char* sensor_filename, char* template_filename) {
    struct server_sensor sensor;
    server_sensor_create(&sensor, sensor_filename);

    struct List list;
    list_create(&list);
    
    

    struct server_socket socket;
    server_socket_create(&socket, port);

    if (!server_socket_start(&socket)) return 1;

    bool was_last_client_valid = true;
    char* temperature;

    while (true) {
        if (was_last_client_valid) {
            temperature = server_sensor_read(&sensor);
        }
        if ( server_sensor_off(&sensor) ){
            break;
        }
        int s = server_socket_accept_client(&socket);
        if (s == -1) {
            return 1;
        }
        was_last_client_valid = process_client(&list, temperature,\
                                         template_filename, &socket);
        server_socket_disable_client(&socket);
        if (was_last_client_valid) free(temperature);
    }

    list_print(&list);

    list_destroy(&list);
    
    server_socket_destroy(&socket);
    server_sensor_destroy(&sensor);
    //} end




    return 0;
}
