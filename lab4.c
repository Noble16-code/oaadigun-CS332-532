#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFSIZE 4096

int main(int argc, char *argv[]) {
    int sourceFile, destFile;
    long int n;
    char buf[BUFFSIZE];
    
    // Check command line arguments
    if (argc != 3) {
        printf("Usage: %s <destination_file> <source_file>\n", argv[0]);
        exit(-1);
    }
    
    // Check if filenames are the same
    if (strcmp(argv[1], argv[2]) == 0) {
        printf("Error: Source and destination filenames cannot be the same\n");
        exit(-1);
    }
    
    // Open source file for reading
    sourceFile = open(argv[2], O_RDONLY);
    if (sourceFile == -1) {
        printf("Error: Cannot open source file '%s'\n", argv[2]);
        exit(-1);
    }
    
    // Open destination file for appending (create if doesn't exist)
    destFile = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (destFile == -1) {
        printf("Error: Cannot open destination file '%s'\n", argv[1]);
        close(sourceFile);
        exit(-1);
    }
    
    // Copy contents from source to destination
    while ((n = read(sourceFile, buf, BUFFSIZE)) > 0) {
        if (write(destFile, buf, n) != n) {
            printf("Error writing to destination file\n");
            close(sourceFile);
            close(destFile);
            exit(-1);
        }
    }
    
    if (n < 0) {
        printf("Error reading from source file\n");
        close(sourceFile);
        close(destFile);
        exit(-1);
    }
    
    // Close files
    close(sourceFile);
    close(destFile);
    
    printf("Successfully concatenated '%s' to '%s'\n", argv[2], argv[1]);
    return 0;
}