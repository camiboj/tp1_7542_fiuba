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
#include "server_tamplate.h"

bool process_client(struct List* list, char* temperature, \
                    struct server_template *template, \
                    struct server_socket* socket) {
    bool status = true;
    char* buffer = server_socket_receive_message(socket);
    if ( !buffer ) return false;

    struct server_req_proc processor;
    status = req_proc_create(&processor, buffer);
    if ( !status ) {
        free(buffer);
        return false;
    }
    char* answer = req_porc_method_resource(&processor);
    if ( !answer ) return false;
    server_socket_send_message(socket, answer, strlen(answer));

    if (! req_porc_is_method_resource_valid(&processor)) {
        req_proc_destroy(&processor);
        free(buffer);
        free(answer);
        return false;
    }
    char* us_ag = req_porc_user_agent(&processor);
    if (!us_ag) return false;
    status = list_insert(list, us_ag);
    if ( !status ) {
        free(buffer);
        free(answer);
        free(us_ag);
        return false;
    }
    
    char* reply = server_template_cat(template, temperature);
    if ( !reply ) {
        free(buffer);
        free(answer);
        free(us_ag);
        return false;
    }
    
    server_socket_send_message(socket, reply, strlen(reply));
    
    req_proc_destroy(&processor);
    free(reply);
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
    
    struct server_sensor sensor;
    if ( !server_sensor_create(&sensor, sensor_filename) ) return 1; 
    
    struct List list;
    list_create(&list);

    struct server_socket socket;
    if ( !server_socket_create(&socket, port) ) {
        server_sensor_destroy(&sensor);
        list_destroy(&list);
        return 1;
    }
    if ( !server_socket_start(&socket) ) {
        server_socket_destroy(&socket);
        server_sensor_destroy(&sensor);
        list_destroy(&list);
        return 1;
    }

    struct server_template template;    
    if ( !server_template_create(&template, template_filename) ) {
        server_socket_destroy(&socket);
        server_sensor_destroy(&sensor);
        list_destroy(&list);
        return 1;
    }
    bool was_last_client_valid = true;
    char* temperature;
    bool is_there_an_error = false;

    while (true) {
        if (was_last_client_valid) {
            temperature = server_sensor_read(&sensor);
        }
        if ( server_sensor_off(&sensor) ){
            break;
        }
        if ( !temperature ) {
            is_there_an_error = true;
            break;
        }
        int s = server_socket_accept_client(&socket);
        if (s == -1) {
            is_there_an_error = true;
            break;
        }
        was_last_client_valid = process_client(&list, temperature,\
                                         &template, &socket);
        server_socket_disable_client(&socket);
        if (was_last_client_valid) free(temperature);
    }

    
    list_print(&list);

    list_destroy(&list);
    server_socket_destroy(&socket);
    server_sensor_destroy(&sensor);
    server_template_destoy(&template);
    if (is_there_an_error) {
        return 1;
    }

    return 0;
}
