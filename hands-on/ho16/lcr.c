// lcr.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_NODES 5

// Unique IDs for the 5 nodes
int node_uids[NUM_NODES] = {12, 5, 23, 8, 15};

void get_fifo_name(int src, int dst, char* buf) {
    sprintf(buf, "/tmp/fifo_lcr_%d_%d", src, dst);
}

void node_process(int id) {
    int my_uid = node_uids[id];
    int next_id = (id + 1) % NUM_NODES;
    int prev_id = (id - 1 + NUM_NODES) % NUM_NODES;
    
    char in_fifo[64], out_fifo[64];
    get_fifo_name(prev_id, id, in_fifo);
    get_fifo_name(id, next_id, out_fifo);
    
    // Open OUT fifo for reading/writing to avoid open() block
    int fd_out = open(out_fifo, O_RDWR);
    int fd_in = open(in_fifo, O_RDWR);
    
    // In LCR, process starts by sending its own UID
    char msg[64];
    sprintf(msg, "UID:%d", my_uid);
    printf("[NODO %d - UID %d] Inizia. Invia '%s' a Nodo %d\n", id, my_uid, msg, next_id);
    write(fd_out, msg, strlen(msg)+1);
    
    int is_participant = 1;
    int leader_known = 0;
    
    while (!leader_known) {
        char buf[64];
        int n = read(fd_in, buf, sizeof(buf));
        if (n > 0) {
            sleep(1); // Rallentatore per visualizzazione didattica
            if (strncmp(buf, "LEADER:", 7) == 0) {
                int leader_uid;
                sscanf(buf, "LEADER:%d", &leader_uid);
                if (leader_uid == my_uid) {
                    printf("[NODO %d - UID %d] L'annuncio LEADER ha fatto il giro. TERMINO.\n", id, my_uid);
                    leader_known = 1;
                } else {
                    printf("[NODO %d - UID %d] Acclamazione ricevuta! Leader e' %d. Inoltro e TERMINO.\n", id, my_uid, leader_uid);
                    write(fd_out, buf, strlen(buf)+1);
                    leader_known = 1;
                }
            } else if (strncmp(buf, "UID:", 4) == 0) {
                int recv_uid;
                sscanf(buf, "UID:%d", &recv_uid);
                
                if (recv_uid > my_uid) {
                    printf("[NODO %d - UID %d] Ricevuto UID %d MAGGIORE. Inoltro e divento passivo.\n", id, my_uid, recv_uid);
                    write(fd_out, buf, strlen(buf)+1);
                    is_participant = 0;
                } else if (recv_uid < my_uid) {
                    printf("[NODO %d - UID %d] Ricevuto UID %d MINORE. Scarto.\n", id, my_uid, recv_uid);
                } else {
                    // recv_uid == my_uid
                    printf(">>> [NODO %d - UID %d] HO RICEVUTO IL MIO UID! DIVENTO LEADER! <<<\n", id, my_uid);
                    sprintf(msg, "LEADER:%d", my_uid);
                    write(fd_out, msg, strlen(msg)+1);
                }
            }
        }
    }
    
    close(fd_in);
    close(fd_out);
    exit(0);
}

int main() {
    printf("--- LCR Algorithm (Ring of 5 Nodes) ---\n\n");
    
    for (int i=0; i<NUM_NODES; i++) {
        char fname[64];
        get_fifo_name(i, (i+1)%NUM_NODES, fname);
        unlink(fname);
        mkfifo(fname, 0666);
    }
    
    for (int i=0; i<NUM_NODES; i++) {
        if (fork() == 0) {
            node_process(i);
        }
    }
    
    for (int i=0; i<NUM_NODES; i++) wait(NULL);
    
    printf("\n--- Esecuzione Terminata. Pulizia FIFO... ---\n");
    for (int i=0; i<NUM_NODES; i++) {
        char fname[64];
        get_fifo_name(i, (i+1)%NUM_NODES, fname);
        unlink(fname);
    }
    return 0;
}
