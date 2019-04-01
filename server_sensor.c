#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "server_sensor.h"

#define SIZE_OF_TEMPERATURE 2
#define MAX_LEN_TEMPERATURE_MESSAGE 200
#define SIZE_TO_REPLACE 9


bool server_sensor_create(struct server_sensor* self, char* filename) {
    FILE* file = fopen(filename, "r+b");
    if (!file) {
	    return false;
    }
    self->file = file;
    return true;
}


char* server_sensor_read(struct server_sensor* self) {
    unsigned short int read;
    size_t len = fread((void*)&read, SIZE_OF_TEMPERATURE, 1, self->file);
    if (!len) return NULL;
    float temperature = (float) ntohs(read);
    temperature = (temperature - 2000) / 100;
    char* message = malloc(MAX_LEN_TEMPERATURE_MESSAGE);
    snprintf(message, SIZE_TO_REPLACE,"%.2f", temperature);
    return message;
}

bool server_sensor_off(struct server_sensor* self) {
    return feof(self->file);
}


void server_sensor_destroy(struct server_sensor* self) {
    fclose(self->file);
}
