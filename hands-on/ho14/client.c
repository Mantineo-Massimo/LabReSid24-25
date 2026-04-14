// client.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

void print_help() {
    printf("Available commands:\n");
    printf("  ADD <id> <amount> <reason>\n");
    printf("  UPDATE <id> <amount> <reason>\n");
    printf("  DELETE <id>\n");
    printf("  LIST\n");
    printf("  QUIT\n");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[2048];
    char response[2048];
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        return 1;
    }
    
    printf("--- Connected to Bank Server ---\n");
    print_help();
    
    while (1) {
        printf("BANK> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        
        if (strncmp(buffer, "QUIT", 4) == 0) {
            send(sock, buffer, strlen(buffer), 0);
            break;
        }
        
        send(sock, buffer, strlen(buffer), 0);
        int read_size = recv(sock, response, 2047, 0);
        if (read_size > 0) {
            response[read_size] = '\0';
            printf("%s", response);
        } else {
            printf("Server Disconnected.\n");
            break;
        }
    }
    
    close(sock);
    return 0;
}
