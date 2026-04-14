#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int child_done = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* thread_child(void* arg) {
    (void)arg; // Evita warning
    printf("Thread Figlio: Iniziato. Faccio un po' di lavoro...\n");
    sleep(2); // "Lavoro" simulato
    printf("Thread Figlio: Lavoro terminato. Avviso il padre...\n");
    
    // Segnala che ha finito
    pthread_mutex_lock(&lock);
    child_done = 1;
    pthread_cond_signal(&cond); // Sveglia il padre in attesa
    pthread_mutex_unlock(&lock);
    
    // Attenzione: senza pthread_join le risorse del thread rimangono
    // logicamente "zombie" fino all'uscita del main se non è creato come detached,
    // ma la traccia chiede solo la simulazione temporale della wait.
    pthread_exit(NULL);
}

int main() {
    pthread_t child;
    printf("Padre: Creo il thread figlio.\n");
    
    // Creazione
    if (pthread_create(&child, NULL, thread_child, NULL) != 0) {
        perror("Errore creazione thread");
        exit(1);
    }
    
    // Attesa del figlio (Simulazione di pthread_join)
    printf("Padre: In attesa della terminazione del figlio...\n");
    
    pthread_mutex_lock(&lock);
    while (child_done == 0) {
        // Rilascia il lock e si addormenta finché non riceve un signal
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);
    
    printf("Padre: Il figlio ha confermato la terminazione (Join simulato con successo).\n");
    
    return 0;
}
