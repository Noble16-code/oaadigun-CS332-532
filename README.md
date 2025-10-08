# CS 332 Lab 6 – Standard I/O Streams and File Operations

## Description
This program reads data from `listings.csv`, stores it in C structures, sorts it by `host_name` and `price` using `qsort()`, and writes the sorted data to new files.

## Files
- `lab6.c`: Main C source file
- `listings.csv`: Input file
- `sorted_by_host.csv`: Output file sorted by host name
- `sorted_by_price.csv`: Output file sorted by price

## Compilation
```bash
gcc lab6.c -o lab6
