/*
 * Compile:  gcc server.c -o server -lpthread
 * Run:      ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_USERS 3

/* Simple protocol */

typedef struct {
    char username[50];
    char password[50];
} Credential;

Credential valid_users[MAX_USERS] = {
    {"alice", "alice123"},
    {"bob",   "bob123"},
    {"carol", "carol123"}
};

pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

void log_event(const char* event) {
    pthread_mutex_lock(&log_lock);
    printf("[LOG] %s\n", event);
    pthread_mutex_unlock(&log_lock);
}

int authenticate(const char* username, const char* password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(valid_users[i].username, username) == 0 &&
            strcmp(valid_users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Basic input validation: reject empty or overly long/garbage input */
int is_valid_input(const char* input) {
    int len = strlen(input);
    if (len == 0 || len >= BUFFER_SIZE - 1) return 0;
    for (int i = 0; i < len; i++) {
        if (input[i] == '\r') return 0; // reject stray control chars
    }
    return 1;
}

void* handle_client(void* arg) {
    int client_fd = *((int*)arg);
    free(arg);

    char buffer[BUFFER_SIZE];
    int authenticated = 0;
    char logged_user[50] = "unknown";
    char log_msg[200];

    snprintf(log_msg, sizeof(log_msg), "Client connected (fd=%d)", client_fd);
    log_event(log_msg);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            snprintf(log_msg, sizeof(log_msg), "Client disconnected (user=%s)", logged_user);
            log_event(log_msg);
            break; // client disconnected or error
        }

        // Strip trailing newline
        buffer[strcspn(buffer, "\n")] = '\0';

        if (!is_valid_input(buffer)) {
            send(client_fd, "ERROR Invalid input\n", 21, 0);
            continue;
        }

        char command[20], arg1[50], arg2[50];
        memset(command, 0, sizeof(command));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        sscanf(buffer, "%19s %49s %49s", command, arg1, arg2);

        if (strcmp(command, "AUTH") == 0) {
            if (authenticate(arg1, arg2)) {
                authenticated = 1;
                strncpy(logged_user, arg1, sizeof(logged_user) - 1);
                send(client_fd, "OK Authenticated\n", 18, 0);
                snprintf(log_msg, sizeof(log_msg), "User '%s' authenticated successfully", arg1);
                log_event(log_msg);
            } else {
                send(client_fd, "ERROR Invalid credentials\n", 27, 0);
                snprintf(log_msg, sizeof(log_msg), "Failed auth attempt for '%s'", arg1);
                log_event(log_msg);
            }
        }
        else if (strcmp(command, "MSG") == 0) {
            if (!authenticated) {
                send(client_fd, "ERROR Not authenticated\n", 25, 0);
                continue;
            }
            char response[BUFFER_SIZE + 50];
            snprintf(response, sizeof(response), "OK Server received: %s\n", buffer + 4);
            send(client_fd, response, strlen(response), 0);

            snprintf(log_msg, sizeof(log_msg), "Message from '%s': %s", logged_user, buffer + 4);
            log_event(log_msg);
        }
        else if (strcmp(command, "QUIT") == 0) {
            send(client_fd, "OK Goodbye\n", 11, 0);
            snprintf(log_msg, sizeof(log_msg), "User '%s' quit", logged_user);
            log_event(log_msg);
            break;
        }
        else {
            send(client_fd, "ERROR Unknown command\n", 23, 0);
        }
    }

    close(client_fd);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow quick restart of server on same port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    printf("Valid users: alice/alice123, bob/bob123, carol/carol123\n\n");

    // 4. Accept clients in a loop, spawn a thread per client
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue; // don't crash the server on one bad accept
        }

        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_fd_ptr) != 0) {
            perror("Thread creation failed");
            close(client_fd);
            free(client_fd_ptr);
            continue;
        }
        pthread_detach(tid); // clean up thread resources automatically on exit
    }

    close(server_fd);
    return 0;
}
