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
#include "common_socket.h"
#include "server_sensor.h"
#include "server_request_processor.h"
#include "server_tamplate.h"

#define MAX_LEN_BUF 2000 

void recive(struct socket* socket, char* buffer, size_t len) {    
    memset(buffer, 0, len);
    int received = 0;
    int status = 0;
    bool is_there_an_error = false;
    while (received < len && !is_there_an_error) {
        status = socket_receive_some(socket, buffer + received, \
                    len - received);
        if (status == 0) { 
            is_there_an_error = true;
        } else if (status < 0) { 
            is_there_an_error = true;
        } else {
            received += status;
        }
    }
}


bool process_client(struct List* list, char* temperature, \
                    struct template *template, \
                    struct socket* socket) {
    bool status = true;
    char buffer[MAX_LEN_BUF];
    recive(socket, buffer, MAX_LEN_BUF);


    struct req_proc processor;
    status = req_proc_create(&processor, buffer);
    if ( !status ) {
        return false;
    }
    char* answer = req_porc_method_resource(&processor);
    if ( !answer ) return false;
    socket_send_all(socket, strlen(answer), answer);

    if (! req_porc_is_method_resource_valid(&processor)) {
        req_proc_destroy(&processor);

        free(answer);
        return false;
    }
    char* us_ag = req_porc_user_agent(&processor);
    if (!us_ag) return false;
    status = list_insert(list, us_ag);
    if ( !status ) {
        free(answer);
        free(us_ag);
        return false;
    }
    template_send_cat(template, temperature);
    
    req_proc_destroy(&processor);

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
    
    struct sensor sensor;
    if ( !sensor_create(&sensor, sensor_filename) ) return 1; 
    
    struct List list;
    list_create(&list);

    struct socket socket;
    socket_create(&socket,NULL, port);

    if ( !socket_start(&socket) ) {
        socket_destroy(&socket);
        sensor_destroy(&sensor);
        list_destroy(&list);
        return 1;
    }

    if ( ! socket_connect_with_clients(&socket) ) {
        socket_destroy(&socket);
        sensor_destroy(&sensor);
        list_destroy(&list);
    }

    struct template template;    
    if ( !template_create(&template, template_filename, &socket) ) {
        socket_destroy(&socket);
        sensor_destroy(&sensor);
        list_destroy(&list);
        return 1;
    }
    
    bool is_there_an_error = false;

  bool was_last_client_valid = true;
    char* temperature;
  
    while (true) {
        if (was_last_client_valid) {
            temperature = sensor_read(&sensor);
        }
        if ( !does_the_sensor_still_have_temperatures(&sensor) ){
            break;
        }
        if ( !temperature ) {
            is_there_an_error = true;
            break;
        }
        int s = socket_accept_client(&socket);
        if (s == -1) {
            is_there_an_error = true;
            break;
        }
        was_last_client_valid = process_client(&list, temperature,\
                                         &template, &socket);
        socket_disable_client(&socket);
        if (was_last_client_valid) free(temperature);
    }

    printf("# Estadisticas de visitantes\n");
    list_print(&list);

    template_destroy(&template);
    sensor_destroy(&sensor);
    socket_destroy(&socket);
    list_destroy(&list);

    if (is_there_an_error) {
        return 1;
    }

    return 0;
}
