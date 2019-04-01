#ifndef SERVER_REQUEST_PROCESSOR_H
#define SERVER_REQUEST_PROCESSOR_H
#include <stdbool.h>


struct server_req_proc {
    char* request;
    bool is_method_resource_valid;
};


void req_proc_create(struct server_req_proc* self, char* request);
void req_proc_destroy(struct server_req_proc* self);
char* req_porc_method_resource(struct server_req_proc* self);
bool req_porc_is_method_resource_valid(struct server_req_proc* self);
char* req_porc_user_agent(struct server_req_proc* self);

#endif
