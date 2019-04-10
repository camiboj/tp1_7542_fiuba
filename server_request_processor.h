#ifndef SERVER_REQUEST_PROCESSOR_H
#define SERVER_REQUEST_PROCESSOR_H
#include <stdbool.h>


struct req_proc {
    char* request;
    bool is_method_resource_valid;
};


bool req_proc_create(struct req_proc* self, char* request);
void req_proc_destroy(struct req_proc* self);

/*
Verifica que el método utilizado sea del tipo "GET" 
y el recurso sea "/sensor".
 
Si el método no es "GET", la respuesta será un 
error de tipo "400 Bad request"
Si el recurso no es "/sensor", la respuesta será un error 
de tipo "404 Not found". 
Si el método y recurso son válidos, la respuesta es 
de tipo "200 OK".
*/
char* req_porc_method_resource(struct req_proc* self);
bool req_porc_is_method_resource_valid(struct req_proc* self);

/*
Busca y devuelve el valor del user-agent en el 
request con formato clave:valor. 
*/
char* req_porc_user_agent(struct req_proc* self);

#endif
