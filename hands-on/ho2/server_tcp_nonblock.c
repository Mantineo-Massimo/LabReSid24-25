#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_CLIENTS 30

typedef struct {
    int socket;
    long long f_n1;
    long long f_n2;
    int count;
} client_state;

int main(int argc, char *argv[]) {
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    char buffer[1025];
    fd_set readfds;
    client_state states[MAX_CLIENTS];

    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
        states[i].socket = 0;
        states[i].f_n1 = 1;
        states[i].f_n2 = 0;
        states[i].count = 0;
    }

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set master socket to non-blocking
    int flags = fcntl(master_socket, F_GETFL, 0);
    fcntl(master_socket, F_SETFL, flags | O_NONBLOCK);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    printf("Non-blocking Server listening on port %d\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        // If master_socket is ready, handle new connection
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd: %d, ip: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Set client socket to non-blocking
            int flags = fcntl(new_socket, F_GETFL, 0);
            fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    states[i].socket = new_socket;
                    states[i].f_n1 = 1;
                    states[i].f_n2 = 0;
                    states[i].count = 0;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // Handle IO on client sockets
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Host disconnected, ip: %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_socket[i] = 0;
                    states[i].socket = 0;
                } else {
                    buffer[valread] = '\0';
                    long long received = atoll(buffer);
                    printf("Received: %lld from socket %d\n", received, sd);

                    int is_correct = 0;
                    if (states[i].count == 0) {
                        if (received == 0) is_correct = 1;
                    } else if (states[i].count == 1) {
                        if (received == 1) is_correct = 1;
                    } else if (received == states[i].f_n1 + states[i].f_n2) {
                        is_correct = 1;
                        long long next = states[i].f_n1 + states[i].f_n2;
                        states[i].f_n2 = states[i].f_n1;
                        states[i].f_n1 = next;
                    }

                    char *ack;
                    if (is_correct) {
                        ack = "ACK_FIB_OK";
                        states[i].count++;
                    } else {
                        ack = "ACK_FIB_ERROR";
                    }

                    send(sd, ack, strlen(ack), 0);
                    printf("Acknowledgment sent to socket %d: %s\n", sd, ack);

                    if (!is_correct) {
                        printf("Sequence error. Closing connection on socket %d.\n", sd);
                        close(sd);
                        client_socket[i] = 0;
                        states[i].socket = 0;
                    }
                }
            }
        }
    }

    return 0;
}
