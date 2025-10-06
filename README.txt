LAb04
Description:
This program concatenates the contents of the second file to the first file
provided as command-line arguments.

Compilation:
$ gcc -Wall -o lab4 lab4.c

Execution:
$ ./lab4 <destination_file> <source_file>

Example:
$ ./lab4 file1.txt file2.txt

This will append the contents of file2.txt to the end of file1.txt.

Features:
- Checks for correct number of command-line arguments
- Prevents concatenating a file to itself
- Handles file opening errors gracefully
- Uses efficient buffered I/O operations
- Preserves file permissions on the destination file


Error Handling:
- If arguments are missing, displays usage message
- If source and destination files are the same, displays error
- If files cannot be opened, displays appropriate error message
- If read/write operations fail, displays error and exits

