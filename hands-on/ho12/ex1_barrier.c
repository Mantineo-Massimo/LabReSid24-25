#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5 // Numero di thread

int counter = 0;
sem_t mutex;
sem_t barrier_sem;

void* thread_task(void* arg) {
    long id = (long)arg;
    
    printf("Thread %ld: Inizializzazione... in attesa alla barriera.\n", id);
    sleep(1 + id); // Simuliamo tempi di arrivo diversi
    
    // Sezione critica per l'incremento del counter
    sem_wait(&mutex);
    counter++;
    printf("Thread %ld: Arrivato alla barriera (%d/%d).\n", id, counter, N);
    
    if (counter == N) {
        // L'ultimo thread arrivato sblocca tutti gli altri
        printf("Thread %ld: E' l'ultimo! Sblocco la barriera per tutti.\n", id);
        for (int i = 0; i < N - 1; i++) {
            sem_post(&barrier_sem);
        }
        sem_post(&mutex);
    } else {
        // I thread non finali arrivati si mettono in attesa
        sem_post(&mutex);
        sem_wait(&barrier_sem);
    }
    
    // Al di qua della barriera i thread partono tutti simultaneamente
    printf("Thread %ld: Ha superato la barriera e sta eseguendo il suo compito!\n", id);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[N];
    
    // Inizializza i semafori
    sem_init(&mutex, 0, 1);
    sem_init(&barrier_sem, 0, 0);
    
    // Creazione dei thread
    for (long i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, thread_task, (void*)i);
    }
    
    // Join dei thread
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Distruzione dei semafori
    sem_destroy(&mutex);
    sem_destroy(&barrier_sem);
    
    printf("Tutti i thread hanno terminato.\n");
    return 0;
}
