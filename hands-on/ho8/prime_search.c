#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_NUMBER 1000000
#define MAX_THREAD 4

// Struttura dati per passare i parametri ai thread
typedef struct {
    int start;
    int end;
    int count;
    int id;
} thread_data_t;

// Funzione fornita dal PDF per la verifica della primalita'
bool is_prime(int n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Funzione eseguita dai thread
void* search_primes(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    data->count = 0;

    for (int i = data->start; i <= data->end; i++) {
        if (is_prime(i)) {
            data->count++;
        }
    }

    printf("Thread %d: Ricerca completata nell'intervallo [%d, %d]. Primi trovati: %d\n", 
           data->id, data->start, data->end, data->count);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[MAX_THREAD];
    thread_data_t tdata[MAX_THREAD];
    int chunk_size = MAX_NUMBER / MAX_THREAD;
    int total_primes = 0;

    printf("Inizio ricerca numeri primi fino a %d con %d thread...\n\n", MAX_NUMBER, MAX_THREAD);

    // Creazione dei thread
    for (int i = 0; i < MAX_THREAD; i++) {
        tdata[i].id = i;
        tdata[i].start = (i * chunk_size) + 1;
        tdata[i].end = (i + 1) * chunk_size;
        
        // Gestione dell'ultimo chunk in caso MAX_NUMBER non sia divisibile per MAX_THREAD
        if (i == MAX_THREAD - 1) {
            tdata[i].end = MAX_NUMBER;
        }

        if (pthread_create(&threads[i], NULL, search_primes, (void*)&tdata[i]) != 0) {
            perror("Chiamata pthread_create fallita");
            exit(EXIT_FAILURE);
        }
    }

    // Join dei thread e conteggio totale
    for (int i = 0; i < MAX_THREAD; i++) {
        pthread_join(threads[i], NULL);
        total_primes += tdata[i].count;
    }

    printf("\nRicerca completata. Totale numeri primi trovati: %d\n", total_primes);

    return 0;
}
