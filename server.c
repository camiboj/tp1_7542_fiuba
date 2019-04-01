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

#define MAX_LEN_FILENAME 100
#define MAX_LEN_REQUEST 2000

#define METHOD_OFFSET 0
#define METHOD_ERROR_MESSAGE "HTTP/1.1 400 Bad request\n"
#define LEN_METHOD 3
#define CORRECT_METHOD "GET"

#define RESOURCE_OFFSET 4
#define RESOURCE_ERROR_MESSAGE "HTTP/1.1 404 Not found\n"

#define LEN_RESOURCE 7
#define CORRECT_RESOURCE "/sensor"

#define METHRES_SUCCESS_MESSAGE "HTTP/1.1 200 OK\n\n"


#define MAX_LEN_ANSWER 16

#define USER_AGENT_KEY "User-Agent:"
#define USER_AGENT_VAL_OFFSET 12
#define END_USER_AGENT_VAL "\n"
#define MAX_LEN_USER_AGENT_VALUE 200

#define MAX_LEN_LINE 100

#define SIZE_OF_TEMPERATURE 2

#define TO_REPLACE "{{datos}}"
#define SIZE_TO_REPLACE 9
#define MAX_LEN_REPLY 2000

#define MAX_LEN_TEMPERATURE_MESSAGE 200

#define MAX_LEN_BUF 2000 




//Verifica que el comando str se encuentre en request 
//De ser así devuelve 0
//en caso contrariodevuelve 1.
int str_check(const char* request, size_t len,const char* str) {//, char* err) {
    for (int i = 0; i < len; ++i) { 
	    if (request[i] != str[i]) {
	        return 1;	    
	    }
    }
    return 0;
}
 
//Verifica que el método utilizado sea del tipo "GET" 
//y el recurso sea "/sensor".
// 
//Si el método no es "GET", la respuesta será un 
//error de tipo "400 Bad request", y si el
//recurso no es "/sensor", la respuesta será un error 
//de tipo "404 Not found".  y devuelve 2
//Si el método y recurso son válidos, la respuesta es 
//de tipo "200 OK" y devuelve 0
enum error {METHRES_SUCCESS, METHOD_ERROR, RESOURCE_ERROR};

int method_resource_processor(char* request) {
    char* position = request + METHOD_OFFSET;
    if (str_check(position, LEN_METHOD, CORRECT_METHOD)) {
	    return METHOD_ERROR;
    }
    position = request + RESOURCE_OFFSET;
    if (str_check(position, LEN_RESOURCE, CORRECT_RESOURCE)) {
	    return RESOURCE_ERROR;
    }
    return METHRES_SUCCESS;
}



void user_agent_processor(char* request, struct List *list) {
    char* key_start = strstr(request, USER_AGENT_KEY);
    if (!key_start) {
        return;
    }
    char* value_start = key_start + USER_AGENT_VAL_OFFSET;
    size_t len_value = 0;
    while (value_start[len_value] && \
           strcmp(value_start + len_value, END_USER_AGENT_VAL)) {
               len_value++;
    }
    char value[MAX_LEN_USER_AGENT_VALUE]; //Do not use variable-length arrays
    snprintf(value, MAX_LEN_USER_AGENT_VALUE -1 , "%s", value_start);
    char* value_end = strstr(value, "\n");
    if (value_end) {
    *   value_end = '\0';
    }
    list_insert(list, value);
}


int send_message(int skt, char *buf, int size) {
    int sent = 0;
    int s = 0;
    bool is_the_socket_valid = true;

    while (sent < size && is_the_socket_valid) {
        s = send(skt, &buf[sent], size-sent, MSG_NOSIGNAL);
      
        if (s == 0) {
            is_the_socket_valid = false;
        } else if (s < 0) {
            is_the_socket_valid = false;
        } else {
            sent += s;
        }
    }

    if (is_the_socket_valid) {
        return sent;
    } else {
        return -1;
    }
}

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

void receive_message(int skt, char* buf, int size){
    int received = 0;
    int s = 0;
    bool is_the_socket_valid = true;
    while (received < size && is_the_socket_valid) {
        s = recv(skt, buf + received, size - received, MSG_NOSIGNAL);
        if (s == 0) { 
            is_the_socket_valid = false;
        } else if (s < 0) { 
            is_the_socket_valid = false;
        } else {
            received += s;
        }
    }
}

bool process_client(int skt, struct List* list,\
                     char* temperature, char* tamplate_filename) {
    bool is_method_resource_valid = true;
    
    char* possible_answers[] = {METHRES_SUCCESS_MESSAGE, METHOD_ERROR_MESSAGE,\
                               RESOURCE_ERROR_MESSAGE};

    

    char buf[MAX_LEN_BUF];
    memset(buf, 0, MAX_LEN_BUF);
    receive_message(skt, buf, MAX_LEN_BUF -1);

    int answer = method_resource_processor(buf);
    is_method_resource_valid = ! answer;
    send_message(skt, possible_answers[answer], \
                 strlen(possible_answers[answer]));
    if (! is_method_resource_valid) return false;
    user_agent_processor(buf, list);
    
    char reply[MAX_LEN_REPLY];
    copy_and_complet(tamplate_filename, reply, MAX_LEN_REPLY, temperature);
    send_message(skt, reply, strlen(reply));
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
    
    //start
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
        int peerskt = server_socket_accept_client(&socket);
        if (peerskt == -1) {
            return 1;
        }
        was_last_client_valid = process_client(peerskt, &list,\
                                   temperature, template_filename);
        shutdown(peerskt, SHUT_RDWR); 
        close(peerskt);
        if (was_last_client_valid) free(temperature);
    }

    list_print(&list);

    list_destroy(&list);
    
    server_socket_destroy(&socket);
    server_sensor_destroy(&sensor);
    //end
    return 0;
}
