/*
CS 332/532 – Lab 3
Homework: Insertion sort with strings using dynamic memory allocation.

How to compile (on CS Linux systems because im on windows):
    gcc -o hwins hwins.c

How to run:
    ./hwins

Tested on: moat.cs.uab.edu (CS Linux systems)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations
void readStrings(char **arr, int n);
void displayStrings(char **arr, int n);
void sortStrings(char **arr, int n);

// Function definitions
void readStrings(char **arr, int n) {
    char buffer[100]; // temp storage
    for (int i = 0; i < n; i++) {
        printf("Enter string %d: ", i + 1);
        scanf("%99s", buffer); // avoid overflow
        arr[i] = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
        strcpy(arr[i], buffer);
    }
}

void displayStrings(char **arr, int n) {
    printf("[");
    for (int i = 0; i < n - 1; i++) {
        printf("%s, ", arr[i]);
    }
    printf("%s]\n", arr[n - 1]);
}

void sortStrings(char **arr, int n) {
    for (int i = 1; i < n; i++) {
        char *key = arr[i];
        int j = i - 1;
        while (j >= 0 && strcmp(arr[j], key) > 0) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int main() {
    int N;
    printf("Enter number of strings: ");
    scanf("%d", &N);

    char **arr = (char**) malloc(N * sizeof(char*));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    readStrings(arr, N);

    printf("Original array: ");
    displayStrings(arr, N);

    sortStrings(arr, N);

    printf("Sorted array: ");
    displayStrings(arr, N);

    // Free memory
    for (int i = 0; i < N; i++) {
        free(arr[i]);
    }
    free(arr);

    return 0;
}
