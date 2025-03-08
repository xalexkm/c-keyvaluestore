#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
	
#define TABLE_SIZE 100
#define PORT 8111
#define DATA_FILE "kv_store.txt"
#define BUFFER_SIZE 1024

typedef struct KeyValue {
	char key[50];
	char value[50];
	struct KeyValue* next;
} KeyValue;

KeyValue* hashTable[TABLE_SIZE];

unsigned int hash(char* key) {
	unsigned long int value = 0;
	for (int i = 0; key[i] != '\0'; i++) {
		value = value * 37 + key[i];
	}
	printf("Getting hash for %s: %lu\n", key, value);
	return value % TABLE_SIZE;
}

void save_to_file() {
	FILE* file = fopen(DATA_FILE, "w");
	if (!file) return;

	for (int i = 0; i < TABLE_SIZE; i++) {
		KeyValue* current = hashTable[i];
		while (current) {
			fprintf(file, "%s=%s\n", current->key, current->value);
			current = current->next;
		}
	}
	fclose(file);
}

void set(char* key, char* value) {
    unsigned int index = hash(key);
    KeyValue* current = hashTable[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strcpy(current->value, value);
            return;
        }
        current = current->next;
    }
    KeyValue* newPair = malloc(sizeof(KeyValue));
    strcpy(newPair->key, key);
    strcpy(newPair->value, value);
    newPair->next = hashTable[index];
    hashTable[index] = newPair;
	save_to_file();
}

void load_from_file() {
	FILE* file = fopen(DATA_FILE, "r");
	if (!file) return;
	
	char key[50], value[50];
	while (fscanf(file, "%49[^=]=%49[^\n]\n", key, value) == 2) {
		set(key, value);
	}
	fclose(file);
}

char* get(char* key) {
	unsigned int index = hash(key);
	KeyValue* current = hashTable[index];

	while (current) {
		if (strcmp(current->key, key) == 0) {
			return current->value;
		}
		current = current->next;
	}
	return NULL;
}

void delete(char* key) {
	unsigned int index = hash(key);
	KeyValue* current = hashTable[index];
	KeyValue* prev = NULL;
	while (current) {
		if (strcmp(current->key, key) == 0) {
			if (prev) prev->next = current->next;
			else hashTable[index] = current->next;
			free(current);
			return;
		}
		prev = current;
    	current = current->next;
	}
	save_to_file();
}

void handle_client(int client_socket) {
	char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);

    char command[10], key[50], value[50];
    sscanf(buffer, "%s %s %s", command, key, value);

    char response[BUFFER_SIZE];

    if (strcmp(command, "SET") == 0) {
        set(key, value);
        snprintf(response, BUFFER_SIZE, "OK\n");
    } else if (strcmp(command, "GET") == 0) {
        char* result = get(key);
        snprintf(response, BUFFER_SIZE, "%s\n", result ? result : "NOT FOUND");
    } else if (strcmp(command, "DELETE") == 0) {
        delete(key);
        snprintf(response, BUFFER_SIZE, "DELETED\n");
    } else {
        snprintf(response, BUFFER_SIZE, "ERROR: Invalid Command\n");
    }

    write(client_socket, response, strlen(response));
    close(client_socket);	
}

void start_server() {
	int server_socket, client_socket;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_size = sizeof(client_addr);

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		perror("Socket creation failed");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Bind failed");
		exit(1);
	}

	if (listen(server_socket, 5) < 0) {
		perror("Listen failed");
		exit(1);
	}
	
	printf("Server started on port %d...\n", PORT);

	while (1) {
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
		if (client_socket < 0) {
			perror("Client connection failed");
			continue;
		}
		handle_client(client_socket);
	}
	
	close(server_socket);
}
	
int main() {
	start_server();
	return 0;
}
