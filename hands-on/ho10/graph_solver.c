#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Struttura per passare gli operandi ai thread */
typedef struct {
    int op1;
    int op2;
    char operation;
} thread_args_t;

/* Funzioni per le operazioni elementari */
void* compute(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    int* result = malloc(sizeof(int));

    switch (args->operation) {
        case '*': *result = args->op1 * args->op2; break;
        case '+': *result = args->op1 + args->op2; break;
        case '-': *result = args->op1 - args->op2; break;
        default: *result = 0; break;
    }

    printf("[Thread %c] Eseguo: %d %c %d = %d\n", 
           (args->operation == '*' ? 'A' : (args->operation == '+' ? 'B' : 'C')),
           args->op1, args->operation, args->op2, *result);

    pthread_exit(result);
}

int main() {
    pthread_t tA, tB, tC;
    thread_args_t argsA = {2, 6, '*'};  /* Nodo A: 2 * 6 */
    thread_args_t argsB = {1, 4, '+'};  /* Nodo B: 1 + 4 */
    thread_args_t argsC = {5, 2, '-'};  /* Nodo C: 5 - 2 */

    int *resA, *resB, *resC;

    printf("Risoluzione equazione: y = (2 * 6) + (1 + 4) * (5 - 2)\n");
    printf("Avvio dei thread per le operazioni indipendenti...\n\n");

    /* Lancio i primi 3 thread in parallelo */
    pthread_create(&tA, NULL, compute, &argsA);
    pthread_create(&tB, NULL, compute, &argsB);
    pthread_create(&tC, NULL, compute, &argsC);

    /* Sincronizzazione: attendo i risultati */
    pthread_join(tA, (void**)&resA);
    pthread_join(tB, (void**)&resB);
    pthread_join(tC, (void**)&resC);

    printf("\nRisultati parziali ottenuti. Calcolo i nodi dipendenti...\n");

    /* Nodo D = B * C (Dipende dai thread B e C) */
    int resD = (*resB) * (*resC);
    printf("[Main] Nodo D (B * C): %d * %d = %d\n", *resB, *resC, resD);

    /* Nodo E = A + D (Risultato finale) */
    int final_y = (*resA) + resD;
    printf("[Main] Risultato Finale y (A + D): %d + %d = %2d\n", *resA, resD, final_y);

    /* Pulizia memoria */
    free(resA);
    free(resB);
    free(resC);

    return 0;
}
