#ifndef SERVER_SENSOR_H
#define SERVER_SENSOR_H
#include <stdbool.h>

struct sensor {
    FILE* file;
};

bool sensor_create(struct sensor* self, char* filename);
/*
Se ocupa de leer de del archivo binario file una temperatura 
almacenada en 16 bits y formato big-endian. 
La misma la interpreta de la siguiente forma: Temperatura = (datos - 2000)/100
*/
char* sensor_read(struct sensor* self);

//comunica si quedan o no temperaturas por leer.
bool does_the_sensor_still_have_temperatures(struct sensor* self);

void sensor_destroy(struct sensor* self);

#endif

