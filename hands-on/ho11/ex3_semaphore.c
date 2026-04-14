#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 5

sem_t max_concurrency_sem;

void* thread_function(void* arg) {
    long thread_id = (long)arg;
    
    printf("Thread %ld is waiting to enter the critical section...\n", thread_id);
    sem_wait(&max_concurrency_sem);
    
    // Critical Section
    printf("Thread %ld has ENTERED the critical section.\n", thread_id);
    sleep(2); // Simulate some work
    printf("Thread %ld is LEAVING the critical section.\n", thread_id);
    
    sem_post(&max_concurrency_sem);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;

    // Initialize semaphore to 2, so at most 2 threads can be in the critical section concurrently
    if (sem_init(&max_concurrency_sem, 0, 2) != 0) {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }

    for (long t = 0; t < NUM_THREADS; t++) {
        rc = pthread_create(&threads[t], NULL, thread_function, (void*)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    sem_destroy(&max_concurrency_sem);
    printf("All threads completed.\n");
    return 0;
}
