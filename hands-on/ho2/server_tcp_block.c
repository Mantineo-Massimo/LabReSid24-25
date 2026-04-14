#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Passive listening
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Blocking Server listening on port %d\n", PORT);

    while (1) {
        printf("Waiting for connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted.\n");

        long long f_n2 = 0, f_n1 = 1;
        int count = 0;
        int valread;

        while ((valread = read(new_socket, buffer, 1024)) > 0) {
            buffer[valread] = '\0';
            long long received = atoll(buffer);
            printf("Received: %lld (count: %d)\n", received, count);

            int is_correct = 0;
            if (count == 0) {
                if (received == 0) is_correct = 1;
            } else if (count == 1) {
                if (received == 1) is_correct = 1;
            } else {
                if (received == f_n2 + f_n1) {
                    is_correct = 1;
                    long long next = f_n2 + f_n1;
                    f_n2 = f_n1;
                    f_n1 = next;
                }
            }

            char *ack;
            if (is_correct) {
                ack = "ACK_FIB_OK";
                count++;
            } else {
                ack = "ACK_FIB_ERROR";
            }

            send(new_socket, ack, strlen(ack), 0);
            printf("Acknowledgment sent: %s\n", ack);

            if (!is_correct) {
                printf("Sequence error. Closing connection.\n");
                break;
            }
        }

        close(new_socket);
        printf("Connection closed.\n");
    }

    close(server_fd);
    return 0;
}
