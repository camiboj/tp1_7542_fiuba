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



#include "server_list.h"

#define MAX_LEN_FILENAME 100
#define MAX_LEN_REQUEST 2000

#define METHOD_OFFSET 0
#define METHOD_ERROR "400 Bad request"
#define LEN_METHOD 3
#define CORRECT_METHOD "GET"

#define RESOURCE_OFFSET 4
#define RESOURCE_ERROR "404 Not found"

#define LEN_RESOURCE 7
#define CORRECT_RESOURCE "/sensor"

#define METHRES_SUCCESS "200 OK"

#define UA_KEY "User-Agent:"
#define UA_VAL_OFFSET 12
#define END_UA_VAL "\n"
#define MAX_LEN_UA_VALUE 50


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





int recv_message(int skt, char *buf, int size) {
   int received = 0;
   int s = 0;
   bool is_the_socket_valid = true;

   while (received < size && is_the_socket_valid) {
      s = recv(skt, &buf[received], size-received, MSG_NOSIGNAL);
      
      if (s == 0) { // nos cerraron el socket :(
         is_the_socket_valid = false;
      }
      else if (s < 0) { // hubo un error >(
         is_the_socket_valid = false;
      }
      else {
         received += s;
      }
   }

   if (is_the_socket_valid) {
      return received;
   }
   else {
      return -1;
   }
}

int send_message(int skt, char *buf, int size) {
   int sent = 0;
   int s = 0;
   bool is_the_socket_valid = true;

   while (sent < size && is_the_socket_valid) {
      s = send(skt, &buf[sent], size-sent, MSG_NOSIGNAL);
      
      if (s == 0) {
         is_the_socket_valid = false;
      }
      else if (s < 0) {
         is_the_socket_valid = false;
      }
      else {
         sent += s;
      }
   }

   if (is_the_socket_valid) {
      return sent;
   }
   else {
      return -1;
   }
}


/*
   Hacemos un procesamiento a un password.
   Como liberamos la memoria el password es eliminado asi que es seguro, no? 
                                                que podria salir mal? 
*/
#define REPEATS 64   //agregar mas repeticiones si es necesario
void process(char *password) {
   int i;

   const char *msg = "Your secret password is: %s!";
   int msg_len = strlen(msg) - 2; // no contar el "%s" (2 bytes)
   int password_len = strlen(password);

   char *buf[REPEATS];
   int buf_size = password_len + msg_len + 1;

   for (i = 0; i < REPEATS; ++i) {
      buf[i] = (char*) malloc(sizeof(char) * buf_size);
   
      snprintf(buf[i], buf_size, msg, password);
      buf[i][buf_size-1] = 0;
   }

   for (i = 0; i < REPEATS; ++i) {
      free(buf[i]);
   }
}

#define MAX_SMALL_BUF_LEN 3   //2 bytes

int serv_start(int argc, char *argv[]) {
   int s = 0;
   unsigned short len = 0;
   bool continue_running = true;
   bool is_the_accept_socket_valid = true;
   
   struct addrinfo hints;
   struct addrinfo *ptr;

   int skt, peerskt = 0;
   int val;

   char small_buf[MAX_SMALL_BUF_LEN];
   char *tmp;

   if (argc != 3) return 1; 

   process(argv[2]);

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
   hints.ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
   hints.ai_flags = AI_PASSIVE;     /* AI_PASSIVE for server           */

   s = getaddrinfo(NULL, argv[1], &hints, &ptr);

   if (s != 0) { 
      printf("Error in getaddrinfo: %s\n", gai_strerror(s));
      return 1;
   }

   skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

   if (skt == -1) {
      printf("Error: %s\n", strerror(errno));
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
   s = listen(skt, 20);
   if (s == -1) {
      printf("Error: %s\n", strerror(errno));
      close(skt);
      return 1;
   }

   while (continue_running) {
      peerskt = accept(skt, NULL, NULL);   // aceptamos un cliente
      if (peerskt == -1) {
         printf("Error: %s\n", strerror(errno));
         continue_running = false;
         is_the_accept_socket_valid = false;
      }
   
      else {
         printf("New client\n");
         memset(small_buf, 0, MAX_SMALL_BUF_LEN);
         recv_message(peerskt, small_buf, MAX_SMALL_BUF_LEN-1); 
         
         len = atoi(small_buf);
         printf("Echo %i bytes\n", len);

         if (len == 0) {
            printf("Zero bytes. Bye!\n");
            continue_running = false;
         }

         else {
            tmp = (char*) malloc(sizeof(char) * len);

            printf("Received %i/%i bytes\n", recv_message(peerskt, tmp, len), len);
            printf("Sent %i/%i bytes\n\n", send_message(peerskt, tmp, len), len);
            free(tmp);
         }

         shutdown(peerskt, SHUT_RDWR);
         close(peerskt);
      }
   }
   
   shutdown(skt, SHUT_RDWR);
   close(skt);

   if (is_the_accept_socket_valid) {
      return 1;
   } 
   else { 
      return 0;
   }
}


//******************************************************
//*		            	SERVER               	  	   *
//******************************************************

//Verifica que el comando str se encuentre en request 
//De ser así devuelve 0
//en caso contrario imprime el error err y devuelve 1.
int str_check(const char* request, size_t len,const char* str, char* err) {
    for (int i = 0; i < len; ++i) { 
	if (request[i] != str[i]) {
	    fprintf(stderr, "%s\n", err);
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
//de tipo "404 Not found". 
//Si el método y recurso son válidos, la respuesta es 
//de tipo "200 OK".
int method_resource_ok(char* request){
    char* position = request + METHOD_OFFSET;
    if (str_check(position, LEN_METHOD, CORRECT_METHOD, METHOD_ERROR))
	return 1;
    position = request + RESOURCE_OFFSET;
    if (str_check(position, LEN_RESOURCE, CORRECT_RESOURCE, RESOURCE_ERROR))
	return 1;
    fprintf(stdout, "%s\n", METHRES_SUCCESS);
    return 0;
}



void ua_processor(char* request, struct List *list) {
    char* key_start = strstr(request, UA_KEY);

    char* value_start = key_start + UA_VAL_OFFSET;
    size_t len_value = 1;
    while (value_start[len_value] && \
           strcmp(value_start + len_value, END_UA_VAL)) len_value++;

    char value[len_value+1]; //= malloc(len_value);

    snprintf(value, len_value+1, "%s", value_start);
    fprintf(stdout, "%s\n", value);

    list_insert(list, value);

}

int request_parser(char* request) {
    if (method_resource_ok(request)) return 1;
    struct List list;
    list_create(&list);
    ua_processor(request, &list);
    list_destroy(&list);
    return 0;
}



int main(int argc, char* argv[]) {
    if (argc !=4) {
	fprintf(stderr, "Uso:\n./server <puerto> <input> [<template>]");
	return 1;
    }
    // tomo el nombre del archivo binario
    //char* sensor_filename = argv[2];
    //char* template_filename = argv[3];
    //char* 






    //por ahora saco texto de un arch pasado por stdin
    char filename[MAX_LEN_FILENAME];
    char* status = fgets(filename, MAX_LEN_FILENAME, stdin);
    filename[strlen(filename) -1] = '\0';
    if (!status) {
	fprintf(stderr, "ERROR\n");
	return 1;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
	fprintf(stderr, "ERROR\n");
	return 1;
    }

    char request[MAX_LEN_REQUEST];
    int c;
    int i = 0;
    while (!feof(file) && i < MAX_LEN_REQUEST) {
	c = fgetc(file);
        request[i] = c;
	i++;
    }
    request[i-2] = (int)'\0';
    //fin

    //función que realmente parsea el request HTTP
    
    if (request_parser(request)) {
	fclose(file);
	return 1;
    }

    //fin

    fclose(file);
    return 0;
}
