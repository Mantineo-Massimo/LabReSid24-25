// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_NUM 10
#define MAX_RECORDS 1000

typedef struct {
    int id;
    float amount;
    char reason[256];
    int active;
} Transaction;

Transaction db[MAX_RECORDS];
pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t thread_limiter;

void process_request(char* buffer, char* response) {
    char reason[256];
    int id;
    float amount;
    
    // Default response
    strcpy(response, "INVALID COMMAND\n");

    if (strncmp(buffer, "ADD", 3) == 0) {
        if (sscanf(buffer, "ADD %d %f %[^\n]", &id, &amount, reason) == 3) {
            pthread_mutex_lock(&db_lock);
            int inserted = 0;
            // Check for duplicates
            int duplicate = 0;
            for (int i=0; i<MAX_RECORDS; i++) {
                if (db[i].active && db[i].id == id) {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate) {
                sprintf(response, "ERR: ID %d ALREADY EXISTS\n", id);
            } else {
                for (int i=0; i<MAX_RECORDS; i++) {
                    if (!db[i].active) {
                        db[i].id = id;
                        db[i].amount = amount;
                        strncpy(db[i].reason, reason, 255);
                        db[i].reason[255] = '\0';
                        db[i].active = 1;
                        inserted = 1;
                        break;
                    }
                }
                if (inserted) sprintf(response, "OK: ADDED %d\n", id);
                else strcpy(response, "ERR: DATABASE FULL\n");
            }
            pthread_mutex_unlock(&db_lock);
        }
    }
    else if (strncmp(buffer, "DELETE", 6) == 0) {
        if (sscanf(buffer, "DELETE %d", &id) == 1) {
            pthread_mutex_lock(&db_lock);
            int deleted = 0;
            for (int i=0; i<MAX_RECORDS; i++) {
                if (db[i].active && db[i].id == id) {
                    db[i].active = 0;
                    deleted = 1;
                    break;
                }
            }
            if (deleted) sprintf(response, "OK: DELETED %d\n", id);
            else sprintf(response, "ERR: ID %d NOT FOUND\n", id);
            pthread_mutex_unlock(&db_lock);
        }
    }
    else if (strncmp(buffer, "UPDATE", 6) == 0) {
        if (sscanf(buffer, "UPDATE %d %f %[^\n]", &id, &amount, reason) == 3) {
            pthread_mutex_lock(&db_lock);
            int updated = 0;
            for (int i=0; i<MAX_RECORDS; i++) {
                if (db[i].active && db[i].id == id) {
                    db[i].amount = amount;
                    strncpy(db[i].reason, reason, 255);
                    db[i].reason[255] = '\0';
                    updated = 1;
                    break;
                }
            }
            if (updated) sprintf(response, "OK: UPDATED %d\n", id);
            else sprintf(response, "ERR: ID %d NOT FOUND\n", id);
            pthread_mutex_unlock(&db_lock);
        }
    }
    else if (strncmp(buffer, "LIST", 4) == 0) {
        pthread_mutex_lock(&db_lock);
        strcpy(response, "--- TRANSACTION LIST ---\n");
        int count = 0;
        char temp[512];
        for (int i=0; i<MAX_RECORDS; i++) {
            if (db[i].active) {
                sprintf(temp, "[%d] %.2f - %s\n", db[i].id, db[i].amount, db[i].reason);
                if (strlen(response) + strlen(temp) < 2040) {
                    strcat(response, temp);
                }
                count++;
            }
        }
        if (count == 0) strcpy(response, "LIST EMPTY\n");
        pthread_mutex_unlock(&db_lock);
        strcat(response, "------------------------\n");
    }
}

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    
    char buffer[2048];
    char response[2048];
    int read_size;
    
    // Persistent connection per client terminal
    while ((read_size = recv(client_sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        if (strncmp(buffer, "QUIT", 4) == 0) break;
        
        process_request(buffer, response);
        send(client_sock, response, strlen(response), 0);
    }
    
    close(client_sock);
    sem_post(&thread_limiter); // Free a thread slot
    pthread_exit(NULL);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    sem_init(&thread_limiter, 0, MAX_NUM);
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    listen(server_sock, MAX_NUM);
    
    printf("Bank Server is LISTENING on port %d (Max %d Conns)...\n", PORT, MAX_NUM);
    
    while (1) {
        sem_wait(&thread_limiter); // Wait for a thread slot
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            sem_post(&thread_limiter);
            continue;
        }
        
        printf("Bank Server: New Connection Accepted.\n");
        
        pthread_t thread;
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        
        pthread_create(&thread, NULL, handle_client, (void*)new_sock);
        pthread_detach(thread);
    }
    return 0;
}
