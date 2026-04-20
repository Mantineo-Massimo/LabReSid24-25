#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <semaphore.h>

typedef struct job {
    void (*function)(void*);
    void* arg;
    struct job* next;
} job_t;

typedef struct {
    pthread_t* threads;
    int num_threads;
    job_t* queue_head;
    job_t* queue_tail;
    pthread_mutex_t queue_mutex;
    sem_t queue_sem;
    int shutdown;
} thread_pool_t;

thread_pool_t* thread_pool_init(int num_threads);
void thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg);
void thread_pool_destroy(thread_pool_t* pool);

#endif
