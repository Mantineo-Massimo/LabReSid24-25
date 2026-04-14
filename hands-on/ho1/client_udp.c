#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char *hello = "Hello from client";
  char buffer[1024] = {0};

  // Creazione del socket
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation error");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Conversione dell'indirizzo IP
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    perror("invalid address/ address not supported");
    exit(EXIT_FAILURE);
  }

  // Invio di un messaggio al server
  sendto(sock, hello, strlen(hello), 0, (struct sockaddr *)&serv_addr,
         sizeof(serv_addr));
  printf("Hello message sent\n");

  // Attesa della risposta dal server (comunicazione bidirezionale)
  socklen_t len = sizeof(serv_addr);
  int n = recvfrom(sock, (char *)buffer, 1024, 0, (struct sockaddr *)&serv_addr, &len);
  buffer[n] = '\0';
  printf("Server reply : %s\n", buffer);

  // Chiusura del socket
  close(sock);
  return 0;
}