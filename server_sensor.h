#ifndef SERVER_SENSOR_H
#define SERVER_SENSOR_H
#include <stdbool.h>

struct server_sensor {
    FILE* file;
};

bool server_sensor_create(struct server_sensor* self, char* filename);
char* server_sensor_read(struct server_sensor* self);
bool server_sensor_off(struct server_sensor* self);
void server_sensor_destroy(struct server_sensor* self);

#endif

