#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_EVENTS 10
#define BUF_SIZE 1024

// Function to set a file descriptor to non-blocking mode
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}

int main() {
    int listen_fd, epoll_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];

    // Create the listening socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR to allow immediate restart of the server
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d using epoll()...\n", PORT);

    // Create epoll instance
    if ((epoll_fd = epoll_create1(0)) == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Add listening socket to epoll
    ev.events = EPOLLIN; // Level-triggered by default
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        perror("epoll_ctl: listen_fd");
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE];
    const char *ack_msg = "ACK_EPOLL_OK\n";

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                // Handle new connection
                int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (conn_fd == -1) {
                    perror("accept");
                    continue;
                }

                printf("New connection: [fd:%d] %s:%d\n", conn_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                set_nonblocking(conn_fd);
                ev.events = EPOLLIN; // Using Level-Triggered for robustness
                ev.data.fd = conn_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                    perror("epoll_ctl: conn_fd");
                    close(conn_fd);
                }
            } else if (events[i].events & EPOLLIN) {
                // Handle data from client
                int client_fd = events[i].data.fd;
                ssize_t n = read(client_fd, buffer, BUF_SIZE - 1);
                
                if (n <= 0) {
                    if (n == 0) {
                        printf("Client disconnected: [fd:%d]\n", client_fd);
                    } else {
                        perror("read error");
                    }
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                } else {
                    buffer[n] = '\0';
                    printf("Message from Client [fd:%d]: %s", client_fd, buffer);
                    
                    // Send acknowledgment
                    if (send(client_fd, ack_msg, strlen(ack_msg), 0) == -1) {
                        perror("send");
                    }
                }
            } else if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                // Handle error or hangup
                int client_fd = events[i].data.fd;
                printf("Error on connection [fd:%d]\n", client_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                close(client_fd);
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}
