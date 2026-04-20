/**
 * @file main.c
 * @author Massimo Mantineo (541924) & Piierluca Tino Castorina (545101)
 * @brief Concurrent HTTP Server entry point.
 * @version 1.1
 * @date 2024-2025
 * 
 * @details Initializes the Resource Manager, Thread Pool, and accepts incoming
 *          TCP connections. Delegates HTTP request handling to the Worker Threads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/thread_pool.h"
#include "../include/resource_manager.h"
#include "../include/http_handler.h"
#include "../include/http_handler.h"

#define PORT 8080
#define THREAD_COUNT 10

/**
 * @brief Task struct mapping a socket fd to the synchronized storage cache.
 */
typedef struct {
    int client_socket;
    resource_manager_t* rm;
} client_task_t;

/**
 * @brief Dispatch routine assigned to worker threads.
 * 
 * @param arg Pointer to client_task_t context.
 */
void job_handler(void* arg) {
    client_task_t* task = (client_task_t*)arg;
    if (task) {
        handle_client(task->client_socket, task->rm);
        free(task);
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    resource_manager_t* rm = rm_init();
    thread_pool_t* pool = thread_pool_init(THREAD_COUNT);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        client_task_t* task = (client_task_t*)malloc(sizeof(client_task_t));
        if (!task) {
            perror("Failed to allocate task for client");
            close(client_socket);
            continue;
        }
        task->client_socket = client_socket;
        task->rm = rm;

        thread_pool_add_job(pool, job_handler, task);
    }

    thread_pool_destroy(pool);
    rm_destroy(rm);
    return 0;
}
