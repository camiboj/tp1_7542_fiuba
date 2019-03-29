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

#define MAX_LEN_FILENAME 100
#define MAX_LEN_REQUEST 2000

#define METHOD_OFFSET 0
#define METHOD_ERROR_MESSAGE "400 Bad request\n"
#define LEN_METHOD 3
#define CORRECT_METHOD "GET"

#define RESOURCE_OFFSET 4
#define RESOURCE_ERROR_MESSAGE "404 Not found\n"

#define LEN_RESOURCE 7
#define CORRECT_RESOURCE "/sensor"

#define METHRES_SUCCESS_MESSAGE "200 OK\n\n"


#define MAX_LEN_ANSWER 16

#define USER_AGENT_KEY "User-Agent:"
#define USER_AGENT_VAL_OFFSET 12
#define END_USER_AGENT_VAL "\n"
#define MAX_LEN_USER_AGENT_VALUE 50

#define MAX_LEN_LINE 100

#define SIZE_OF_TEMPERATURE 2

#define TO_REPLACE "{{datos}}"
#define SIZE_TO_REPLACE 9
#define MAX_LEN_REPLY 2000

#define MAX_LEN_TEMPERATURE_MESSAGE 200
#define MAX_WAAITING_CLIENTS 20
/*
   Ejemplo de un echo server.
   Este ejemplo TIENE UN BUG (en realidad son 2 combinados).
   La idea de un echo server es la siguiente:
      1) El cliente se conecta a este server y le envia un numero 
         de 2 digitos (en texto) que representa la longitud del 
         mensaje que le sigue.
      2) El cliente luego envia ese mensaje de esa longitud
      3) El server lee ese mensaje y luego se lo reenvia al cliente.
   Se compila con 
      gcc -std=c99 -o echoserver echoserver.c 
   Se ejecuta como
      ./echoserver PORT PASSWORD
   donde PORT es el nombre de un servicio ("http" por ejemplo) o el numero
   de puerto directamente (80 por ejemplo) 
   PASSWORD es un string que representa algo secreto. No tiene nada que
   ver con el echo server y es borrado de la memoria con free
   Asi que no deberia haber ningun problema en, por ejemplo, que pongas 
   tu password de facebook/clave bancaria, no?
*/




//Verifica que el comando str se encuentre en request 
//De ser así devuelve 0
//en caso contrariodevuelve 1.
int str_check(const char* request, size_t len,const char* str) {//, char* err) {
    for (int i = 0; i < len; ++i) { 
	    if (request[i] != str[i]) {
	        //fprintf(stderr, "%s\n", err);
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
    if (!key_start) return;
    char* value_start = key_start + USER_AGENT_VAL_OFFSET;
    size_t len_value = 1;
    while (value_start[len_value] && \
           strcmp(value_start + len_value, END_USER_AGENT_VAL)) len_value++;

    char value[MAX_LEN_USER_AGENT_VALUE]; //Do not use variable-length arrays

    snprintf(value, len_value, "%s", value_start);
    //fprintf(stdout, "%s\n", value);

    list_insert(list, value);
}


int send_message(int skt, char *buf, int size) {
    //printf("\nsize = %d\n", size);
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

char* sensor_reader(FILE* sensor_file) {
    unsigned short int read;
    size_t len = fread((void*)&read, SIZE_OF_TEMPERATURE, 1, sensor_file);
    if (!len) return NULL;
    //printf("%d\n", read);
    float temperature = (float) ntohs(read);
    temperature = (temperature - 2000) / 100;
    char* message = malloc(MAX_LEN_TEMPERATURE_MESSAGE);
    snprintf(message, SIZE_TO_REPLACE,"%.2f", temperature);
    printf("0: %s\n", message);
    return message;
}


void copy_and_complet(char* tamplate_filename, char* reply, size_t len_reply, char* replacement) {
    
    FILE* tamplate = fopen(tamplate_filename, "r");
    if (!tamplate_filename) {
	    //fprintf(stderr, "ERROR\n");
	    return;
    }
    
    size_t i = 0;
    while(!feof(tamplate) && i < len_reply) {
        reply[i] = (char) fgetc(tamplate);
        i++;
    }
    reply[i-1] = '\0';
    char* to_replace = strstr(reply, TO_REPLACE);
    printf("\nrplacement = %s\n", replacement);
    size_t len_replacement = strlen(replacement);

    
    
    sprintf(to_replace, "%s", replacement);
    //printf("1: %s\n\n", reply);
    if (SIZE_TO_REPLACE > len_replacement) {
        snprintf(&to_replace[len_replacement], strlen(&to_replace[SIZE_TO_REPLACE]),"%s", &to_replace[SIZE_TO_REPLACE]);
    }
    //printf("2: %s\n", reply);
    fclose(tamplate);
}

void process_client(int skt, char *buf, int size, struct List* list, char* temperature, \
                    char* tamplate_filename) {
    int received = 0;
    int s = 0;
    bool is_the_socket_valid = true;
    bool is_method_resource_valid = true;
    
    bool is_first_line = true;
    char* line;
    char* possible_answers[] = {METHRES_SUCCESS_MESSAGE, METHOD_ERROR_MESSAGE,\
                               RESOURCE_ERROR_MESSAGE};

    while (received < size && is_the_socket_valid) {
        s = recv(skt, buf + received, size - received, MSG_NOSIGNAL);
        int answer = 0;
        line = strstr(buf, "\n");
        if (line) {
            if (is_first_line){
                is_first_line = false;
                answer = method_resource_processor(buf);
                is_method_resource_valid = ! answer;
                buf = line + 1;
                send_message(skt, possible_answers[answer], \
                     strlen(possible_answers[answer]));
            } else {
                user_agent_processor(line, list);
            }
        }
        
        if ( !is_method_resource_valid ) {
            return;
        }

        //fprintf(stdout, "%s\n", buf+received);
        if (s == 0) { // nos cerraron el socket :(
            is_the_socket_valid = false;
        } else if (s < 0) { // hubo un error >(
            is_the_socket_valid = false;
        } else {
            received += s;
        }
    }
    //char* temperature = sensor_reader(sensor_file);
    
    char reply[MAX_LEN_REPLY];
    copy_and_complet(tamplate_filename, reply, MAX_LEN_REPLY, temperature);
    send_message(skt, reply, strlen(reply));
    //printf("\nreply sended = %ld \n", strlen(reply));
    //free(temperature);
    //printf("%.2f\n", temperature);
}






#define MAX_LEN_BUF 2000 

int serv_start(char* port, struct List* list, FILE* sensor_file, char* template_filename) {
   int s = 0;
   unsigned short len = 0;
   bool continue_running = true;
   bool is_the_accept_socket_valid = true;
   
   struct addrinfo hints;
   struct addrinfo *ptr;

   int skt, peerskt = 0;
   int val;

   char buf[MAX_LEN_BUF];
   //char *tmp;

   //if (argc != 3) return 1; 

   //process(argv[2]);

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
   hints.ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
   hints.ai_flags = AI_PASSIVE;     /* AI_PASSIVE for server           */

   s = getaddrinfo(NULL, port, &hints, &ptr);

   if (s != 0) { 
      printf("Error in getaddrinfo: %s\n", gai_strerror(s));
      return 1;
   }

   skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

   if (skt == -1) {
      printf("Error1: %s\n", strerror(errno));
      freeaddrinfo(ptr);
      return 1;
   }

   // Activamos la opcion de Reusar la Direccion en caso de que esta
   // no este disponible por un TIME_WAIT
   val = 1;
   s = setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
   if (s == -1) {
      printf("Error: %s\n", strerror(errno));
      close(skt);
      freeaddrinfo(ptr);
      return 1;
   }

   // Decimos en que direccion local queremos escuchar, en especial el puerto
   // De otra manera el sistema operativo elegiria un puerto random
   // y el cliente no sabria como conectarse
   s = bind(skt, ptr->ai_addr, ptr->ai_addrlen);
   if (s == -1) {
      printf("Error: %s\n", strerror(errno));
      close(skt);
      freeaddrinfo(ptr);
      return 1;
   }

   freeaddrinfo(ptr);

   // Cuanto clientes podemos mantener en espera antes de poder acceptarlos?
   s = listen(skt, MAX_WAAITING_CLIENTS);
   if (s == -1) {
      printf("Error: %s\n", strerror(errno));
      close(skt);
      return 1;
   }
    char* temperature;
    while (continue_running) {
        temperature = sensor_reader(sensor_file);
        if (feof(sensor_file)){
            break;
        }
        peerskt = accept(skt, NULL, NULL);   // aceptamos un cliente
        if (peerskt == -1) {
            printf("Error: %s\n", strerror(errno));
            continue_running = false;
            is_the_accept_socket_valid = false;
        } else {
            //printf("New client\n");
            memset(buf, 0, MAX_LEN_BUF);
            process_client(peerskt, buf, MAX_LEN_BUF-1, list, temperature, template_filename); 
            shutdown(peerskt, SHUT_RDWR); 
            close(peerskt);
            free(temperature);
         len = atoi(buf);
         printf("Echoo %i bytes\n", len);
  
      }
   }
   
   shutdown(skt, SHUT_RDWR);
   close(skt);

   if (is_the_accept_socket_valid) {
      return 1;
   } else { 
      return 0;
   }
}


//******************************************************
//*		            	SERVER               	  	   *
//******************************************************

int main(int argc, char* argv[]) {
    if (argc !=4) {
	    fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]\n");
	    return 1;
    }
    
    char* port = argv[1];
    char* sensor_filename = argv[2];
    char* template_filename = argv[3];
    


    FILE* file = fopen(sensor_filename, "r+b");
    if (!file) {
	    //fprintf(stderr, "ERROR\n");
	    return 1;
    }

    
    struct List list;
    list_create(&list);


    
    //while (!feof(file) && i < MAX_LEN_REQUEST) {

        
    
    serv_start(port, &list, file, template_filename);
    //fin
    //list_print(&list);
    list_destroy(&list);
    fclose(file);
    return 0;
}
