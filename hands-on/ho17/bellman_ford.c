// bellman_ford.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <sys/select.h>

#define NUM_NODES 4
#define INF 999
#define TARGET_NODE 0

// Grafo dei Costi C(x,y)
/* Topologia Ponderata (Costi asimmetrici per testare i Minimum Paths):
   Node 0 is the Target.
   (0)-(1) cost: 2
   (0)-(2) cost: 5
   (1)-(2) cost: 1
   (1)-(3) cost: 4
   (2)-(3) cost: 1

   Minimum distances to 0 dynamically:
   0: 0
   1: 2 (via 0)
   2: 3 (via 1) -> 5 via 0 is discarded!
   3: 4 (via 2) -> 6 via 1 is discarded!
*/

int cost_matrix[NUM_NODES][NUM_NODES] = {
    {0, 2, 5, INF}, // Dal nodo 0
    {2, 0, 1, 4},   // Dal nodo 1
    {5, 1, 0, 1},   // Dal nodo 2
    {INF, 4, 1, 0}  // Dal nodo 3
};

void get_fifo_name(int src, int dst, char* buf) {
    sprintf(buf, "/tmp/fifo_bf_%d_%d", src, dst);
}

void msg(int src, int dst, int d) {
    char fname[64];
    get_fifo_name(src, dst, fname);
    int fd = open(fname, O_WRONLY);
    if(fd != -1) {
        char buf[64];
        sprintf(buf, "%d", d);
        write(fd, buf, strlen(buf)+1);
        close(fd);
    }
}

void node_process(int id) {
    int my_dist = (id == TARGET_NODE) ? 0 : INF;
    int next_hop = (id == TARGET_NODE) ? id : -1;
    
    // Predisposizione FIFO ingresso
    int in_fds[NUM_NODES];
    int num_in = 0;
    int from_nodes[NUM_NODES];
    
    for (int i=0; i<NUM_NODES; i++) {
        if (cost_matrix[id][i] != INF && cost_matrix[id][i] != 0) {
            char fname[64];
            get_fifo_name(i, id, fname); // Chi scrive mi parla su questa pipe
            in_fds[num_in] = open(fname, O_RDWR); // RDWR prevents blocking
            from_nodes[num_in] = i;
            num_in++;
        }
    }
    
    // Bootstrap: l'iniziatore target spinge a tutti la sua distanza = 0.
    // Anche gli altri potrebbero spingere INF, ma ignoriamo per risparmiare traffico.
    if (id == TARGET_NODE) {
        printf("[NODO %d - TARGET] Distanza zero locale. Segnalo Inizio alle adiacenze.\n", id);
        for(int i=0; i<NUM_NODES; i++) {
            if (cost_matrix[id][i] != INF && cost_matrix[id][i] != 0) {
                msg(id, i, my_dist);
            }
        }
    }
    
    int active = 1;
    int timeout_counts = 0;
    
    while(active && timeout_counts < 3) { // Aspetta timeout per dichiarare 'Convergenza'
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = 0;
        
        for (int i=0; i<num_in; i++) {
            FD_SET(in_fds[i], &read_fds);
            if(in_fds[i] > max_fd) max_fd = in_fds[i];
        }
        
        struct timeval tv;
        tv.tv_sec = 2; // Rallentamento e attesa
        tv.tv_usec = 0;
        
        int ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        if (ret > 0) {
            timeout_counts = 0; // Azzera
            for (int i=0; i<num_in; i++) {
                if (FD_ISSET(in_fds[i], &read_fds)) {
                    char buf[64];
                    int n = read(in_fds[i], buf, sizeof(buf));
                    if (n > 0) {
                        int reported_dist = atoi(buf);
                        int sender = from_nodes[i];
                        int my_cost_to_sender = cost_matrix[id][sender];
                        int new_dist = reported_dist + my_cost_to_sender;
                        
                        sleep(1); // Per la didattica Console (slowed down)
                        
                        if (new_dist < my_dist) {
                            printf("[NODO %d] D-V UPDATE! Vett. %d > %d (Nuovo hop: Nodo %d). Espando l'onda d'urto...\n", 
                                   id, my_dist, new_dist, sender);
                            my_dist = new_dist;
                            next_hop = sender;
                            
                            // Espansione di "Bellman Ford": ricalcolo positivo, avvisa chiunque
                            for(int j=0; j<NUM_NODES; j++) {
                                if (cost_matrix[id][j] != INF && cost_matrix[id][j] != 0 && j != sender) {
                                    msg(id, j, my_dist);
                                }
                            }
                        } else {
                            // Nessun vantaggio. Distance Vector invariato.
                        }
                    }
                }
            }
        } else {
            timeout_counts++;
        }
    }
    
    printf("--- [NODO %d ESTINTO/CONVERSO] Shortest Path Totale => %d (via Nodo %d) ---\n", id, my_dist, next_hop);
    
    for (int i=0; i<num_in; i++) {
        close(in_fds[i]);
    }
    exit(0);
}

int main() {
    printf("===== Bellman-Ford Algoritmo di Distance Vector Distribuito =====\n");
    printf("Obiettivo: Cammini ottimali da ogni Nodo verso il NODO TARGET (0)\n\n");
    
    // Inizializza l'ambiente Pipe Globale Malloc-Free
    for(int i=0; i<NUM_NODES; i++) {
        for(int j=0; j<NUM_NODES; j++) {
            if (cost_matrix[i][j] != INF && cost_matrix[i][j] != 0) {
                char fname[64];
                get_fifo_name(i, j, fname);
                unlink(fname);
                mkfifo(fname, 0666);
            }
        }
    }
    
    // Crea Agenti Indipendenti
    for(int i=0; i<NUM_NODES; i++) {
        if(fork() == 0) {
            node_process(i);
        }
    }
    
    // Barrier su Termine
    for(int i=0; i<NUM_NODES; i++) wait(NULL);
    
    printf("\n==== Convergenza Assoluta Raggiunta. Discesa IPC ==== \n");
    for(int i=0; i<NUM_NODES; i++) {
        for(int j=0; j<NUM_NODES; j++) {
            if (cost_matrix[i][j] != INF && cost_matrix[i][j] != 0) {
                char fname[64];
                get_fifo_name(i, j, fname);
                unlink(fname);
            }
        }
    }
    
    return 0;
}
