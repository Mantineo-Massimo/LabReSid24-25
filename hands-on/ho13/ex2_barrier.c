#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 4

int count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* thread_task(void* arg) {
    long id = (long)arg;
    
    printf("Thread %ld: Faccio operazioni preliminari...\n", id);
    sleep(1 + id); // Simulazione arrivi sfasati
    
    // Raggiunta la barriera
    pthread_mutex_lock(&lock);
    count++;
    printf("Thread %ld: Arrivato alla barriera. (Presenti: %d/%d)\n", id, count, N);
    
    if (count == N) {
        // Sono l'ultimo, risveglio tutti in un colpo solo
        printf("Thread %ld: Sono l'ultimo arrivato! Sblocco tutti quanti!\n", id);
        pthread_cond_broadcast(&cond);
    } else {
        // Non siamo tutti, attendo addormentato
        while (count < N) {
            pthread_cond_wait(&cond, &lock);
        }
    }
    pthread_mutex_unlock(&lock);
    
    // Oltrepassata la barriera, si eseguono queste operazioni contemporaneamente
    printf("Thread %ld: Barriera superata! Avvio simultaneo.\n", id);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[N];
    
    // Creazione thread
    for (long i = 0; i < N; i++) {
        if (pthread_create(&threads[i], NULL, thread_task, (void*)i) != 0) {
            perror("Errore creazione thread");
            exit(1);
        }
    }
    
    // In questo caso, visto che non fa parte dell'esercizio vietarlo,
    // usiamo la normale pthread_join per pulire
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Tutti i thread terminati.\n");
    return 0;
}
