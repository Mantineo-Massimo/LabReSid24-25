#include "../include/thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

static void* thread_worker(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;

    while (1) {
        sem_wait(&pool->queue_sem);

        pthread_mutex_lock(&pool->queue_mutex);
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }

        job_t* job = pool->queue_head;
        if (job) {
            pool->queue_head = job->next;
            if (pool->queue_head == NULL) {
                pool->queue_tail = NULL;
            }
        }
        pthread_mutex_unlock(&pool->queue_mutex);

        if (job) {
            (*(job->function))(job->arg);
            free(job);
        }
    }

    return NULL;
}

thread_pool_t* thread_pool_init(int num_threads) {
    thread_pool_t* pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    pool->num_threads = num_threads;
    pool->queue_head = NULL;
    pool->queue_tail = NULL;
    pool->shutdown = 0;

    pthread_mutex_init(&pool->queue_mutex, NULL);
    sem_init(&pool->queue_sem, 0, 0);

    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, thread_worker, (void*)pool);
    }

    return pool;
}

void thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg) {
    job_t* job = (job_t*)malloc(sizeof(job_t));
    job->function = function;
    job->arg = arg;
    job->next = NULL;

    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->queue_tail == NULL) {
        pool->queue_head = job;
        pool->queue_tail = job;
    } else {
        pool->queue_tail->next = job;
        pool->queue_tail = job;
    }
    pthread_mutex_unlock(&pool->queue_mutex);

    sem_post(&pool->queue_sem);
}

void thread_pool_destroy(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_mutex_unlock(&pool->queue_mutex);

    for (int i = 0; i < pool->num_threads; i++) {
        sem_post(&pool->queue_sem);
    }

    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    pthread_mutex_destroy(&pool->queue_mutex);
    sem_destroy(&pool->queue_sem);
    free(pool);
}
