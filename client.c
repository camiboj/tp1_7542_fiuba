#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN_FILENAME 50
#define MAX_LEN_FILE 200


//******************************************************
//*			CLIENTE		  	      *
//******************************************************


//client ​ <host> <port> ​ [ ​ <filename>​ ]

//Donde ​ <host>​ y ​ <port>​ son la dirección IPv4 o ​ hostname
//y el puerto o servicio donde el servidor estará escuchando 
//la conexión TCP.

//<filename>​ es un argumento opcional que indica el ​ archivo 
//de texto ​ con el request a enviar.

//Si el argumento no es pasado, el cliente leerá el request de la ​ entrada estándar ​ .*/

int procesar_archivo(char* filename, size_t path_len, char* path) {
    FILE* file = fopen(filename, "r");
    if (!file) {
	//fprintf(stderr, "ERROR\n");
	return 1;
    }

    int i = 0;
    while(!feof(file) && i < path_len) {
	int c = fgetc(file);
        path[i] = c;
	i++;
    }
    path[i] = (int)'\0';

    fclose(file);
    return 0;
}


int main(int argc, char* argv[]) {
    if (argc != 3 && argc !=4) {
	fprintf(stderr, "ERROR\n");
	return 1;
    }    

    char* filename = malloc(MAX_LEN_FILENAME);
    if (argc == 3) {
	char* status = fgets(filename, MAX_LEN_FILENAME, stdin);
	filename[strlen(filename) -1] = '\0';
	if (!status) return 1;
    }
    if (argc == 4) {
	snprintf(filename, MAX_LEN_FILENAME, "%s", argv[3]);
	fprintf(stdout,"%s\n", argv[3]);
    }

    char* path = malloc(MAX_LEN_FILE);
    if (procesar_archivo(filename, MAX_LEN_FILE, path)) {
	free(path);
        free(filename);
	return 1;
    }
 
    free(path);
    free(filename);
}
