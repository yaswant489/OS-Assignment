
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 5
#define LOG_FILE "audit.log"

/* User Authentication */
typedef struct {
    char username[50];
    char password[50];
    char role[10]; // "owner", "group", "other"
} User;

User users[MAX_USERS] = {
    {"alice", "alice123", "owner"},
    {"bob",   "bob123",   "group"},
    {"carol", "carol123", "other"}
};
int user_count = 3;

User* authenticate(const char* username, const char* password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            return &users[i];
        }
    }
    return NULL; // authentication failed
}

/* Audit Logging */

void log_action(const char* username, const char* action, const char* filename, const char* result) {
    FILE* log = fopen(LOG_FILE, "a");
    if (log == NULL) {
        printf("ERROR: Could not open audit log.\n");
        return;
    }

    time_t now = time(NULL);
    char timestamp[26];
    strcpy(timestamp, ctime(&now));
    timestamp[strlen(timestamp) - 1] = '\0'; // remove trailing newline

    fprintf(log, "[%s] USER=%s ACTION=%s FILE=%s RESULT=%s\n",
            timestamp, username, action, filename, result);
    fclose(log);
}

/* Permission System */
/* Simplified rwx model, similar to Unix chmod (owner/group/other) */

typedef struct {
    int owner_read, owner_write, owner_exec;
    int group_read, group_write, group_exec;
    int other_read, other_write, other_exec;
} Permissions;

Permissions default_permissions() {
    Permissions p = {1,1,1,  1,0,1,  0,0,0}; // owner: rwx, group: r-x, other:
    return p;
}

int check_permission(Permissions p, const char* role, char action) {
    // action: 'r' = read, 'w' = write, 'x' = execute
    if (strcmp(role, "owner") == 0) {
        if (action == 'r') return p.owner_read;
        if (action == 'w') return p.owner_write;
        if (action == 'x') return p.owner_exec;
    } else if (strcmp(role, "group") == 0) {
        if (action == 'r') return p.group_read;
        if (action == 'w') return p.group_write;
        if (action == 'x') return p.group_exec;
    } else { // other
        if (action == 'r') return p.other_read;
        if (action == 'w') return p.other_write;
        if (action == 'x') return p.other_exec;
    }
    return 0;
}

void print_permissions(Permissions p) {
    printf("Permissions: owner(%c%c%c) group(%c%c%c) other(%c%c%c)\n",
        p.owner_read?'r':'-', p.owner_write?'w':'-', p.owner_exec?'x':'-',
        p.group_read?'r':'-', p.group_write?'w':'-', p.group_exec?'x':'-',
        p.other_read?'r':'-', p.other_write?'w':'-', p.other_exec?'x':'-');
}

/* File Operations */

int create_file(User* user, Permissions perms, const char* filename) {
    if (!check_permission(perms, user->role, 'w')) {
        printf("DENIED: %s lacks write permission to create %s\n", user->username, filename);
        log_action(user->username, "CREATE", filename, "DENIED");
        return 0;
    }
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        log_action(user->username, "CREATE", filename, "ERROR");
        return 0;
    }
    fclose(f);
    printf("SUCCESS: %s created %s\n", user->username, filename);
    log_action(user->username, "CREATE", filename, "SUCCESS");
    return 1;
}

int write_file(User* user, Permissions perms, const char* filename, const char* content) {
    if (!check_permission(perms, user->role, 'w')) {
        printf("DENIED: %s lacks write permission for %s\n", user->username, filename);
        log_action(user->username, "WRITE", filename, "DENIED");
        return 0;
    }
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        log_action(user->username, "WRITE", filename, "ERROR");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    printf("SUCCESS: %s wrote to %s\n", user->username, filename);
    log_action(user->username, "WRITE", filename, "SUCCESS");
    return 1;
}

int read_file(User* user, Permissions perms, const char* filename) {
    if (!check_permission(perms, user->role, 'r')) {
        printf("DENIED: %s lacks read permission for %s\n", user->username, filename);
        log_action(user->username, "READ", filename, "DENIED");
        return 0;
    }
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        printf("ERROR: %s not found\n", filename);
        log_action(user->username, "READ", filename, "ERROR");
        return 0;
    }
    char buffer[1024];
    printf("--- Contents of %s ---\n", filename);
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        printf("%s", buffer);
    }
    printf("\n---------------------------\n");
    fclose(f);
    log_action(user->username, "READ", filename, "SUCCESS");
    return 1;
}

int delete_file(User* user, Permissions perms, const char* filename) {
    if (!check_permission(perms, user->role, 'w')) {
        printf("DENIED: %s lacks permission to delete %s\n", user->username, filename);
        log_action(user->username, "DELETE", filename, "DENIED");
        return 0;
    }
    if (remove(filename) == 0) {
        printf("SUCCESS: %s deleted %s\n", user->username, filename);
        log_action(user->username, "DELETE", filename, "SUCCESS");
        return 1;
    } else {
        printf("ERROR: could not delete %s\n", filename);
        log_action(user->username, "DELETE", filename, "ERROR");
        return 0;
    }
}

/* Encryption / Decryption */
/*
 * Simple XOR cipher for demonstration purposes.
 * NOTE: XOR with a short repeating key is NOT secure for real use
 * (vulnerable to frequency analysis / known-plaintext attacks).
 * A production system should use AES via a library such as OpenSSL.
 * This limitation is discussed in the Security Analysis report.
 */

void xor_encrypt_decrypt(char* data, int len, const char* key) {
    int key_len = strlen(key);
    for (int i = 0; i < len; i++) {
        data[i] = data[i] ^ key[i % key_len];
    }
}

void encrypt_file(User* user, const char* filename, const char* key) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        printf("ERROR: %s not found for encryption\n", filename);
        return;
    }
    char buffer[1024];
    int len = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);

    xor_encrypt_decrypt(buffer, len, key);

    f = fopen(filename, "wb");
    fwrite(buffer, 1, len, f);
    fclose(f);

    printf("SUCCESS: %s encrypted %s\n", user->username, filename);
    log_action(user->username, "ENCRYPT", filename, "SUCCESS");
}

void decrypt_file(User* user, const char* filename, const char* key) {
    // XOR is symmetric: applying it again with the same key reverses it
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        printf("ERROR: %s not found for decryption\n", filename);
        return;
    }
    char buffer[1024];
    int len = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);

    xor_encrypt_decrypt(buffer, len, key);

    f = fopen(filename, "wb");
    fwrite(buffer, 1, len, f);
    fclose(f);

    printf("SUCCESS: %s decrypted %s\n", user->username, filename);
    log_action(user->username, "DECRYPT", filename, "SUCCESS");
}

/* MAIN */

int main() {
    printf("=== Task 3: Secure File Management System ===\n\n");

    // Authentication demo
    printf("--- Authentication ---\n");
    User* current_user = authenticate("alice", "alice123");
    if (current_user == NULL) {
        printf("Authentication FAILED\n");
        return 1;
    }
    printf("Authenticated as: %s (role: %s)\n\n", current_user->username, current_user->role);

    // Permissions demo
    Permissions perms = default_permissions();
    printf("--- File Permissions ---\n");
    print_permissions(perms);
    printf("\n");

    // File operations demo
    printf("--- File Operations ---\n");
    create_file(current_user, perms, "secret.txt");
    write_file(current_user, perms, "secret.txt", "This is confidential information.");
    read_file(current_user, perms, "secret.txt");

    // Try operation as a user without write access
    User* other_user = authenticate("carol", "carol123");
    printf("\n--- Access Attempt by 'other' role user ---\n");
    write_file(other_user, perms, "secret.txt", "Malicious overwrite attempt");
    read_file(other_user, perms, "secret.txt"); // also denied since other has no read

    // Encryption demo
    printf("\n--- Encryption / Decryption ---\n");
    const char* key = "mySecretKey";
    encrypt_file(current_user, "secret.txt", key);
    printf("File is now encrypted (unreadable as plain text).\n");
    decrypt_file(current_user, "secret.txt", key);
    read_file(current_user, perms, "secret.txt");

    // Delete demo
    printf("\n--- Delete Operation ---\n");
    delete_file(current_user, perms, "secret.txt");

    printf("\nAll actions have been recorded in %s\n", LOG_FILE);
    return 0;
}
