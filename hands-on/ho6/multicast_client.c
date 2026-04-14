#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MULTICAST_GROUP "239.0.0.1"
#define PORT 12345
#define MAX_BUFFER 1024

int main() {
    int sockfd;
    struct sockaddr_in local_addr;
    struct ip_mreq multicast_req;
    char buffer[MAX_BUFFER];

    // 1. Creare un socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Settare il socket in SO_REUSEADDR
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 3. Bindare su INADDR_ANY
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 4. Aggiungere il socket al gruppo multicast utilizzando IP_ADD_MEMBERSHIP
    multicast_req.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_req.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_req, sizeof(multicast_req)) < 0) {
        perror("IP_ADD_MEMBERSHIP failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client iscritto al gruppo multicast %s sulla porta %d\n", MULTICAST_GROUP, PORT);
    printf("In attesa di messaggi...\n\n");

    // 5. Ricevere i dati
    while (1) {
        memset(buffer, 0, MAX_BUFFER);
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        
        int n = recvfrom(sockfd, buffer, MAX_BUFFER - 1, 0, 
                        (struct sockaddr *)&sender_addr, &sender_len);
        
        if (n < 0) {
            perror("Recvfrom failed");
            break;
        }

        printf("Ricevuto: %s | (Da: %s:%d)\n", 
               buffer, inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));
    }

    // 6. Abbandonare il gruppo multicast con IP_DROP_MEMBERSHIP
    if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast_req, sizeof(multicast_req)) < 0) {
        perror("IP_DROP_MEMBERSHIP failed");
    }

    close(sockfd);
    return 0;
}
