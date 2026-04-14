// flooding.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <string.h>

#define MAX_AGENTS 4
#define MSG_SIZE 256

// Topologia a Diamante
// 0 connesso a 1, 2
// 1 connesso a 0, 3
// 2 connesso a 0, 3
// 3 connesso a 1, 2
int num_neighbors[MAX_AGENTS] = {2, 2, 2, 2};
int neighbors[MAX_AGENTS][2] = {
    {1, 2}, // Nodo 0
    {0, 3}, // Nodo 1
    {0, 3}, // Nodo 2
    {1, 2}  // Nodo 3
};

// Costruisce il nome univoco della FIFO direzionale
void get_fifo_name(int src, int dst, char* buf) {
    sprintf(buf, "/tmp/fifo_%d_%d", src, dst);
}

// msg(): Invia l'informazione al vicino tramite la named pipe
void msg(int src, int dst, const char* info) {
    char fifo_name[64];
    get_fifo_name(src, dst, fifo_name);
    // Apriamo la pipe in sola scrittura
    int fd = open(fifo_name, O_WRONLY);
    if(fd != -1) {
        write(fd, info, strlen(info) + 1);
        close(fd);
    }
}

// stf() State Transition Function: Gestisce il passaggio di stato e l'inoltro
void stf(int id, int* state, int sender, const char* info) {
    if (*state == 0) { // PASSIVE
        printf("[AGENTE %d] RICEZIONE da Nodo %d: '%s'. TRASIZIONE a DONE e Propagazione.\n", id, sender, info);
        *state = 1; // DONE
        
        // Propaga l'informazione a tutti i vicini ESCLUSO colui che gliel'ha mandata
        for(int i = 0; i < num_neighbors[id]; i++) {
            int neigh = neighbors[id][i];
            if (neigh != sender) {
                printf("    -> [AGENTE %d] Propaga messaggio al Nodo %d\n", id, neigh);
                msg(id, neigh, info);
            }
        }
    } else {
        // È già DONE, ha ricevuto il messaggio da un altro ramo (ciclo del grafo)
        printf("[AGENTE %d] SCARTO msg da Nodo %d (Gia' elaborato).\n", id, sender);
    }
}

// Ciclo Vitale del Processo Agente
void agent_process(int id, int is_initiator) {
    int state = 0; // 0 = PASSIVE, 1 = DONE
    
    // Predisposizione delle pipe in Lettura (O_RDWR serve a non bloccare sull'open in Linux)
    int in_fds[MAX_AGENTS];
    for (int i = 0; i < num_neighbors[id]; i++) {
        char fname[64];
        get_fifo_name(neighbors[id][i], id, fname); // Pipe creata DA vicino A questo nodo
        in_fds[i] = open(fname, O_RDWR);
    }
    
    // Seleziona l'azione iniziale a seconda se è lniziatore
    if (is_initiator) {
        printf("\n=> [AGENTE %d] INIZIATORE: Genero il messaggio di Flooding iniziale.\n", id);
        state = 1; // DONE
        for(int i = 0; i < num_neighbors[id]; i++) {
            int neigh = neighbors[id][i];
            printf("    -> [AGENTE %d] Propaga messaggio al Nodo %d\n", id, neigh);
            msg(id, neigh, "ALLERTA_METEO");
        }
    }
    
    // Multiplexer basico con `select()` per restare in ascolto dalle FIFO afferenti
    int cycles_idle = 0;
    while(cycles_idle < 3) { // Se non riceve nulla per ~3 cicli da 1 sec, si spegne
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = 0;
        
        for (int i = 0; i < num_neighbors[id]; i++) {
            FD_SET(in_fds[i], &read_fds);
            if(in_fds[i] > max_fd) max_fd = in_fds[i];
        }
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        if (ret > 0) {
            for (int i = 0; i < num_neighbors[id]; i++) {
                if (FD_ISSET(in_fds[i], &read_fds)) {
                    char buf[MSG_SIZE];
                    int n = read(in_fds[i], buf, MSG_SIZE);
                    if (n > 0) {
                        stf(id, &state, neighbors[id][i], buf);
                        cycles_idle = 0; // Resettiamo essendo ancora attivo
                    }
                }
            }
        } else {
            cycles_idle++;
        }
    }
    
    printf("[AGENTE %d] Spegnimento.\n", id);
    for (int i = 0; i < num_neighbors[id]; i++) {
        close(in_fds[i]);
    }
    exit(0);
}

int main() {
    printf("--- Avvio Algoritmo di Flooding Centralizzato ---\n");
    printf("Topologia: 4 Nodi a Diamante (0 Initiator)\n\n");
    
    // 1. Il modulo MASTER costruisce prima tutti gli pseudo-device .fifo
    for(int i = 0; i < MAX_AGENTS; i++) {
        for(int j = 0; j < num_neighbors[i]; j++) {
            char fname[64];
            get_fifo_name(i, neighbors[i][j], fname);
            unlink(fname); // Pialla pipe vecchie se rimaste bloccate per errore
            mkfifo(fname, 0666);
        }
    }
    
    // 2. Forka i 4 processi figli (Nodi)
    for(int i = 0; i < MAX_AGENTS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Processo figlio Agente (se i==0 Iniziatore == 1/True)
            agent_process(i, (i == 0)); 
            return 0; // Backup
        }
    }
    
    // 3. Attesa della terminazione di tutti i processi (terminano dopo 3s di Inattività)
    for(int i = 0; i < MAX_AGENTS; i++) {
        wait(NULL);
    }
    
    // 4. Pulizia pseudo file di pipe dal cestino di memoria RAM
    printf("\n--- Flooding Terminato. Pulizia Syscall... ---\n");
    for(int i = 0; i < MAX_AGENTS; i++) {
        for(int j = 0; j < num_neighbors[i]; j++) {
            char fname[64];
            get_fifo_name(i, neighbors[i][j], fname);
            unlink(fname);
        }
    }
    
    printf("Chiusura definitiva del programma.\n");
    return 0;
}
