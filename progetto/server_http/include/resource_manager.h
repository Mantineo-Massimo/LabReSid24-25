#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <pthread.h>

#define MAX_RESERVATIONS 100
#define MAX_STR_LEN 128

typedef struct {
    int id;
    char room_name[MAX_STR_LEN];
    char student_name[MAX_STR_LEN];
    char time_slot[MAX_STR_LEN];
} reservation_t;

typedef struct {
    reservation_t reservations[MAX_RESERVATIONS];
    int count;
    int next_id;
    pthread_mutex_t mutex;
} resource_manager_t;

resource_manager_t* rm_init();
int rm_create(resource_manager_t* rm, const char* room, const char* student, const char* time);
int rm_read_all(resource_manager_t* rm, char* buffer, int buffer_size);
int rm_update(resource_manager_t* rm, int id, const char* room, const char* student, const char* time);
int rm_delete(resource_manager_t* rm, int id);
void rm_destroy(resource_manager_t* rm);

#endif
