#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main() {
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;
  char buffer[1024];
  char *hello = "Hello from server";
  socklen_t len;

  // Creazione del socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation error");
    exit(EXIT_FAILURE);
  }

  // Configurazione dell'indirizzo
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Binding del socket
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // Ciclo infinito per mantenere il server in esecuzione
  while (1) {
    len = sizeof(cliaddr);

  // Ricezione del messaggio dal client
  int n = recvfrom(sockfd, (char *)buffer, 1024, 0, (struct sockaddr *)&cliaddr,
                   &len);
  buffer[n] = '\0';
  printf("Client : %s\n", buffer);

    // Invio della risposta al client
    sendto(sockfd, (const char *)hello, strlen(hello), 0,
           (const struct sockaddr *)&cliaddr, len);
    printf("Hello message sent\n");
  }

  // Chiusura del socket
  close(sockfd);
  return 0;
}