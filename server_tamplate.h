#ifndef SERVER_TEMPLATE_H
#define SERVER_TEMPLATE_H

struct server_template {
    char* text;
    char* to_replace;
};


bool server_template_create(struct server_template *self, char* filename);
char* server_template_cat(struct server_template *self, char* replacement);
void server_template_destoy(struct server_template *self);

#endif
