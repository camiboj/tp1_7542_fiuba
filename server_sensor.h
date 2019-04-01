#ifndef SERVER_SENSOR_H
#define SERVER_SENSOR_H
#include <stdbool.h>

struct server_sensor {
    FILE* file;
};

bool server_sensor_create(struct server_sensor* self, char* filename);
/*
Se ocupa de leer de del archivo binario file una temperatura 
almacenada en 16 bits y formato big-endian. 
La misma la interpreta de la siguiente forma: Temperatura = (datos - 2000)/100
*/
char* server_sensor_read(struct server_sensor* self);

//comunica si quedan o no temperaturas por leer.
bool does_the_sensor_still_have_temperatures(struct server_sensor* self);

void server_sensor_destroy(struct server_sensor* self);

#endif

