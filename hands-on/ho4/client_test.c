#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char *message = "Hello from epoll client";
  char buffer[BUF_SIZE] = {0};

  if (argc > 1) {
    message = argv[1];
  }

  // Create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
    return -1;
  }

  // Connect to server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  // Send message
  send(sock, message, strlen(message), 0);
  printf("Message sent: %s\n", message);

  // Read response
  int n = read(sock, buffer, BUF_SIZE - 1);
  if (n > 0) {
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);
  } else if (n == 0) {
    printf("Server closed the connection.\n");
  } else {
    perror("read");
  }

  close(sock);
  return 0;
}
