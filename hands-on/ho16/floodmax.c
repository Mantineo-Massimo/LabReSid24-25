// floodmax.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_NODES 4
#define DIAMETER 2

// Topologia a diamante
int num_neighbors[NUM_NODES] = {2, 2, 2, 2};
int neighbors[NUM_NODES][2] = {
    {1, 2},
    {0, 3},
    {0, 3},
    {1, 2}
};

// UID dei Nodi
int node_uids[NUM_NODES] = {10, 50, 20, 30};

void get_fifo_name(int src, int dst, char* buf) {
    sprintf(buf, "/tmp/fifo_fm_%d_%d", src, dst);
}

void node_process(int id) {
    int my_uid = node_uids[id];
    int max_uid = my_uid;
    
    int fd_in[NUM_NODES];
    int fd_out[NUM_NODES];
    
    for (int i=0; i<num_neighbors[id]; i++) {
        char fname_in[64], fname_out[64];
        get_fifo_name(neighbors[id][i], id, fname_in);
        get_fifo_name(id, neighbors[id][i], fname_out);
        fd_in[i] = open(fname_in, O_RDWR);
        fd_out[i] = open(fname_out, O_RDWR);
    }
    
    for (int r=1; r<=DIAMETER; r++) {
        printf("[NODO %d] ROUND %d: Mio Max= %d. Invio ai vicini...\n", id, r, max_uid);
        char msg[64];
        sprintf(msg, "%d", max_uid);
        
        for (int i=0; i<num_neighbors[id]; i++) {
            write(fd_out[i], msg, strlen(msg)+1);
        }
        
        int new_max = max_uid;
        for (int i=0; i<num_neighbors[id]; i++) {
            char buf[64];
            int n = read(fd_in[i], buf, sizeof(buf));
            if (n > 0) {
                int recv_uid = atoi(buf);
                if (recv_uid > new_max) {
                    new_max = recv_uid;
                }
            }
        }
        max_uid = new_max;
        sleep(1); // Per la sincronizzazione logica su terminale
    }
    
    if (max_uid == my_uid) {
        printf(">>> [NODO %d] SONO IL LEADER! (UID %d) <<<\n", id, my_uid);
    } else {
        printf("[NODO %d] E' emerso un Leader globale con UID %d. Io mi arrendo.\n", id, max_uid);
    }
    
    for (int i=0; i<num_neighbors[id]; i++) {
        close(fd_in[i]);
        close(fd_out[i]);
    }
    exit(0);
}

int main() {
    printf("--- FloodMax Algorithm (Generic Graph diam=2) ---\n\n");
    
    for (int i=0; i<NUM_NODES; i++) {
        for (int j=0; j<num_neighbors[i]; j++) {
            char fname[64];
            get_fifo_name(i, neighbors[i][j], fname);
            unlink(fname);
            mkfifo(fname, 0666);
        }
    }
    
    for (int i=0; i<NUM_NODES; i++) {
        if (fork() == 0) {
            node_process(i);
        }
    }
    
    for (int i=0; i<NUM_NODES; i++) wait(NULL);
    
    printf("\n--- Esecuzione Terminata. Pulizia FIFO... ---\n");
    for (int i=0; i<NUM_NODES; i++) {
        for (int j=0; j<num_neighbors[i]; j++) {
            char fname[64];
            get_fifo_name(i, neighbors[i][j], fname);
            unlink(fname);
        }
    }
    return 0;
}
