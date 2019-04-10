#ifndef SERVER_TEMPLATE_H
#define SERVER_TEMPLATE_H
#include "common_socket.h"
#define MAX_LEN_REPLY 2000

struct template {
    char text[MAX_LEN_REPLY];
    char* to_replace;
    struct socket* skt;
};


bool template_create(struct template *self, char* filename, struct socket* skt);
void template_send_cat(struct template *self, char* replacement);
void template_destroy(struct template *self);

#endif
