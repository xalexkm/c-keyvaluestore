#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	
#define TABLE_SIZE 100

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
}
	
int main() {
    set("name", "Alex");
	set("age", "30");
	printf("Name: %s\n", get("name"));
	printf("Age: %s\n", get("age"));
	delete("age");
	printf("Age after delete: %s\n", get("age"));
	return 0;
}
