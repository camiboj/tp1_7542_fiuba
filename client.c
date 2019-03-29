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
#define MAX_LEN_FILENAME 100
#define MAX_LEN_FILE 2000



/*
   Este es un ejemplo de juguete y por eso usaremos buffers chicos
   y en el stack. Una aplicacion seria manejaria buffers de size
   variable.
*/
#define REQUEST_MAX_LEN 2000
#define RESPONSE_MAX_LEN 2000

int client_start(size_t request_len, char* request, char* host, char* port) {
    int s = 0;
    bool are_we_connected = false;
    bool is_there_a_socket_error = false;
    bool is_the_remote_socket_closed = false;

   // A quien nos queremos conectar
   //const char *hostname = "www.fi.uba.ar";
   //const char *servicename = "http";
   
    struct addrinfo hints;
    struct addrinfo *result, *ptr;

    int skt = 0;

   //char request[REQUEST_MAX_LEN];
   //int request_len;
    char response[RESPONSE_MAX_LEN];

    int bytes_sent = 0;
    int bytes_recv = 0;

   //if (argc != 2) return 1; 

   /* Creo una especie de filtro para decir que direcciones me interesan
      En este caso, TCP sobre IPv4 para ser usada por un socket 'activo'
      (no es para un server)
   */ 
    memset(&hints, 0, sizeof(struct addrinfo)); //setea en 0
    hints.ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
    hints.ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
    hints.ai_flags = 0;              /* None (or AI_PASSIVE for server) */


   /* Obtengo la (o las) direcciones segun el nombre de host y servicio que
      busco
     
      De todas las direcciones posibles, solo me interesan aquellas que sean
      IPv4 y TCP (segun lo definido en hints)
      
      El resultado lo guarda en result
   */
    s = getaddrinfo(host, port, &hints, &result);

   /* Es muy importante chequear los errores. 
      En caso de uno, usar gai_strerror para traducir el codigo de error (s)
      a un mensaje humanamente entendible.
   */
    if (s != 0) { 
        printf("Error in getaddrinfo: %s\n", gai_strerror(s));
        return 1;
    }

   /* getaddrinfo retorna una **lista** de direcciones. Hay que iterarla
      para encontrar la que mejor se ajusta.
      (generalmente la primera direccion es la mejor, pero dependera de 
      cada caso)
      En este caso voy a probar cada direccion posible. La primera que
      funcione sera la que se usara por el resto del ejemplo.
   */
    for (ptr = result; ptr != NULL && are_we_connected == false;\
        ptr = ptr->ai_next) {
      /* Creamos el socket definiendo la familia (deberia ser AF_INET IPv4),
         el tipo de socket (deberia ser SOCK_STREAM TCP) y el protocolo (0) */
        skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (skt == -1) {
            printf("Error1: %s\n", strerror(errno));
        } else {
            /* Nos conectamos a la fiuba
            ai_addr encapsula la IP y el puerto del server.
            La estructura es automaticamente creada por getaddrinfo */
            s = connect(skt, ptr->ai_addr, ptr->ai_addrlen);
            if (s == -1) {
                printf("Error2: %s\n", strerror(errno));
                close(skt);
            }
            are_we_connected = (s != -1); // nos conectamos?
        }
    }

   /* Finalmente, la **lista** de direcciones debe ser liberada */
   freeaddrinfo(result);

    if (are_we_connected == false) {
        return 1; // nos quedamos sin direcciones validas y 
               //no nos pudimos conectar :(
    }

   /* Armamos el mensaje HTTP para hablar con el sitio web.
      Es un mensaje HTTP minimalista, pero sirve.
      Aqui decimos que pagina queremos leer.
      [Ref http://es.wikipedia.org/wiki/Hypertext_Transfer_Protocol]
    */

   
   /* Enviamos el mensaje intentando enviar todo el mensaje de un solo intento,
      y solo reintentando enviar aquellos bytes que no pudiero entrar */
    while (bytes_sent < request_len && is_there_a_socket_error == false && \
        is_the_remote_socket_closed == false) {
        s = send(skt, &request[bytes_sent], request_len - bytes_sent,\
                 MSG_NOSIGNAL);

        if (s < 0) {  // ups,  hubo un error
            printf("Error3: %s\n", strerror(errno));
            is_there_a_socket_error = true;
        } else if (s == 0) { // nos cerraron el socket :(
            is_the_remote_socket_closed = true;
        } else {
            bytes_sent += s;
        }
    }
    shutdown(skt, SHUT_WR);
   
    if (is_the_remote_socket_closed || is_there_a_socket_error) {
        shutdown(skt, SHUT_RDWR);
        close(skt);
        return 1;
    }

   /* 
      Recibimos el mensaje que viene del sitio web.
      Para simplificar, vamos a leer hasta que el server nos cierre la conecci
      (no es para nada optimo, pero es mas simple)
   */
    while (is_there_a_socket_error == false && \
        is_the_remote_socket_closed == false) {
        s = recv(skt, &response[bytes_recv], \
                 RESPONSE_MAX_LEN - bytes_recv - 1, MSG_NOSIGNAL);

        if (s < 0) {
            printf("Error4: %s\n", strerror(errno));
            is_there_a_socket_error = true;
        } else if (s == 0) {
         // cerraron el socket del otro lado:
         // voy a asumir que nos dieron toda la respuesta 
            is_the_remote_socket_closed = true;
        } else {
            bytes_recv = s; 

            response[bytes_recv] = 0;
            
            printf("%s", response);

            //reusamos el mismo buffer, no me interesa tener toda la 
            //respuesta en memoria
            bytes_recv = 0; 
        }
    }
    printf("\n");
    /* Le decimos a la otra maquina que cerramos la coneccion */
    shutdown(skt, SHUT_RDWR);

    /* Cerramos el socket */
    close(skt);

    if (is_there_a_socket_error) {
        return 1;   // hubo un error, somos consistentes 
                  // y salimos con un codigo de error
    } else {
        return 0;
    }
}



//******************************************************
//*			              CLIENT		       	       *
//******************************************************


//client ​ <host> <port> ​ [ ​ <filename>​ ]

//Donde ​ <host>​ y ​ <port>​ son la dirección IPv4 o ​ hostname
//y el puerto o servicio donde el servidor estará escuchando 
//la conexión TCP.


int file_processor(char* filename, size_t* path_len, char* path) {
    FILE* file = fopen(filename, "r");
    if (!file) {
	    //fprintf(stderr, "ERROR\n");
	    return 1;
    }

    int i = 0;
    while (!feof(file) && i < MAX_LEN_FILE) {
	    int c = fgetc(file);
        path[i] = c;
	    i++;
    }
    path[i-2] = (int)'\0'; //me estaba leyendo un \n y un eof o algo asi
    //fprintf(stdout, "%s\n", path);
    fclose(file);
    *path_len = i - 2;
    return 0;
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
	    if (!status) return 1;
    } else {
	    snprintf(filename, MAX_LEN_FILENAME, "%s", argv[3]);
    }

    char path[MAX_LEN_FILE];
    size_t path_len;
    if (file_processor(filename, &path_len, path)){
	    return 1;
    }

    //char* path = "GET /sensor HTTP/1.1\nUser-Agent: sensor-client";
    //size_t path_len = strlen(path);
    char* host = argv[1];    
    char* port = argv[2];
    int result = client_start(path_len, path, host, port);
    return result;
}
