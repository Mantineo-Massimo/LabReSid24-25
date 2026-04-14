#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8080
#define MAX_FIB 20

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Socket creation
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to server. Sending Fibonacci sequence...\n");

    long long a = 0, b = 1, next;
    for (int i = 0; i < MAX_FIB; i++) {
        char msg[64];
        sprintf(msg, "%lld", a);
        
        printf("Sending: %s\n", msg);
        if (send(sock, msg, strlen(msg), 0) < 0) {
            perror("Send failed");
            break;
        }

        // Wait for acknowledgment
        memset(buffer, 0, 1024);
        int valread = read(sock, buffer, 1024);
        if (valread > 0) {
            printf("Server Ack: %s\n", buffer);
            if (strstr(buffer, "ERROR") != NULL) {
                printf("Sequence error detected by server. Stopping.\n");
                break;
            }
        } else {
            printf("No response from server. Connection closed?\n");
            break;
        }

        // Calculate next Fibonacci
        next = a + b;
        a = b;
        b = next;
        
        // Small sleep to separate messages in the TCP stream for the simple server
        usleep(100000); 
    }

    close(sock);
    printf("Client finished.\n");
    return 0;
}
