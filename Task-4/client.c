
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // 1. Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 2. Connect to server
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);
    printf("Commands: AUTH <user> <pass> | MSG <text> | QUIT\n\n");

    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        // Send command to server
        if (send(sock_fd, input, strlen(input), 0) < 0) {
            perror("Send failed");
            break;
        }

        // Receive response
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Server closed the connection.\n");
            break;
        }
        printf("Server: %s", buffer);

        // Exit loop if user sent QUIT
        if (strncmp(input, "QUIT", 4) == 0) break;
    }

    close(sock_fd);
    return 0;
}
