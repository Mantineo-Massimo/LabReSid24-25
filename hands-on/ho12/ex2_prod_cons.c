#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 15

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

sem_t empty_slots;
sem_t filled_slots;
sem_t mutex;

void* producer(void* arg) {
    long id = (long)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = (id * 100) + i; // Genera un item (es: id 1 produce 100, 101, 102)
        
        // Attende che ci sia almeno uno slot vuoto
        sem_wait(&empty_slots);
        // Attende l'accesso esclusivo al buffer
        sem_wait(&mutex);
        
        // Sezione Crirca: Produzione dell'elemento
        buffer[in] = item;
        printf("Produttore %ld: [PRODOTTO] -> %d in pos %d\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        
        // Rilascia l'accesso al buffer
        sem_post(&mutex);
        // Segnala che c'è un elemento IN PIÚ riempito
        sem_post(&filled_slots);
        
        usleep((rand() % 500) * 1000); // Pausa tra le produzioni
    }
    pthread_exit(NULL);
}

void* consumer(void* arg) {
    long id = (long)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Attende che ci sia almeno un elemento pieno da consumare
        sem_wait(&filled_slots);
        // Attende l'accesso esclusivo al buffer
        sem_wait(&mutex);
        
        // Sezione Critica: Consumo dell'elemento
        int item = buffer[out];
        printf("Consumatore %ld: [CONSUMATO] <- %d in pos %d\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        
        // Rilascia l'accesso al buffer
        sem_post(&mutex);
        // Segnala che c'è uno spazio vuoto IN PIÚ nel buffer
        sem_post(&empty_slots);
        
        usleep((rand() % 800) * 1000); // Pausa (simula elaborazione lenta)
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t prod_thread, cons_thread;
    
    // Inizializza i semafori
    // empty_slots parte dalla dimensione intera del buffer
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    // filled_slots parte da 0 (all'inizio non ci sono elementi)
    sem_init(&filled_slots, 0, 0);
    // mutex inizializzato ad 1 per la mutua esclusione sul buffer
    sem_init(&mutex, 0, 1);
    
    // In questo esempio avviamo 1 Produttore e 1 Consumatore
    // Fanno NUM_ITEMS interazioni
    pthread_create(&prod_thread, NULL, producer, (void*)1);
    pthread_create(&cons_thread, NULL, consumer, (void*)1);
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    // Distruzione
    sem_destroy(&empty_slots);
    sem_destroy(&filled_slots);
    sem_destroy(&mutex);
    
    printf("Esecuzione Completata. Buffer Gestito con Successo.\n");
    return 0;
}
