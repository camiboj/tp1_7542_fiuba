#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdbool.h>
#include "server_request_processor.h"

#define METHOD_OFFSET 0
#define METHOD_ERROR_MESSAGE "HTTP/1.1 400 Bad request\n"
#define LEN_METHOD 3
#define CORRECT_METHOD "GET"

#define RESOURCE_OFFSET 4
#define RESOURCE_ERROR_MESSAGE "HTTP/1.1 404 Not found\n"

#define LEN_RESOURCE 7
#define CORRECT_RESOURCE "/sensor"

#define MAX_LEN_REQUEST 2000 

#define METHOD_ERROR_MESSAGE "HTTP/1.1 400 Bad request\n"
#define RESOURCE_ERROR_MESSAGE "HTTP/1.1 404 Not found\n"
#define METHRES_SUCCESS_MESSAGE "HTTP/1.1 200 OK\n\n"
#define MAX_LEN_MESSAGE 26

#define USER_AGENT_KEY "User-Agent:"
#define USER_AGENT_VAL_OFFSET 12
#define END_USER_AGENT_VAL "\n"
#define MAX_LEN_USER_AGENT_VALUE 200




void req_proc_create(struct server_req_proc* self, char* request) {
    self->request = malloc(MAX_LEN_REQUEST);
    snprintf(self->request, MAX_LEN_REQUEST, "%s", request);
    self->is_method_resource_valid = false;
}

void req_proc_destroy(struct server_req_proc* self) {
    free(self->request);
}

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
char* req_porc_method_resource(struct server_req_proc* self) {
    char* answer = malloc(MAX_LEN_MESSAGE);
    enum error {METHRES_SUCCESS, METHOD_ERROR, RESOURCE_ERROR};
    char* position = self->request + METHOD_OFFSET;
    if (str_check(position, LEN_METHOD, CORRECT_METHOD)) {
	    snprintf(answer, MAX_LEN_MESSAGE, "%s", METHOD_ERROR_MESSAGE);
        self->is_method_resource_valid = false; //no es necesario, 
        return answer;                      //pero si llegase a cambiar
    }                                       //implementacion podria serlo
    position = self->request + RESOURCE_OFFSET;
    if (str_check(position, LEN_RESOURCE, CORRECT_RESOURCE)) {
        snprintf(answer, MAX_LEN_MESSAGE, "%s", RESOURCE_ERROR_MESSAGE);
        self->is_method_resource_valid = false;
	    return answer;
    }
    snprintf(answer, MAX_LEN_MESSAGE, "%s", METHRES_SUCCESS_MESSAGE);
    self->is_method_resource_valid = true;
    return answer;
}


bool req_porc_is_method_resource_valid(struct server_req_proc* self) {
    return self->is_method_resource_valid;
}


char* req_porc_user_agent(struct server_req_proc* self) {
    char* key_start = strstr(self->request, USER_AGENT_KEY);
    if (!key_start) {
        return NULL;
    }
    char* value_start = key_start + USER_AGENT_VAL_OFFSET;
    size_t len_value = 0;
    while (value_start[len_value] && \
           strcmp(value_start + len_value, END_USER_AGENT_VAL)) {
               len_value++;
    }
    char* value = malloc(MAX_LEN_USER_AGENT_VALUE);
    if (!value) return NULL;
    snprintf(value, MAX_LEN_USER_AGENT_VALUE, "%s", value_start);
    char* value_end = strstr(value, "\n");
    if (value_end) { //sino eof => strstr deja \n
        *value_end = '\0';
    }
    return value;
}
