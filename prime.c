/*
CS 332/532 – Lab 01
Oladotun Adigun
oaadigun

How to compile (on CS Linux systems because im on windows):
    gcc -o prime prime.c

How to run:
    ./prime

Tested on: moat.cs.uab.edu (CS Linux systems)

Thiscode reads an integer from input and prints
whether it is prime or not.
*/

#include <stdio.h>

int main(void) {
    int given_number;

    // Read input using scanf
    if (scanf("%d", &given_number) != 1) {
        // If input is not an integer, exit
        return 0;
    }

    
    if (given_number <= 1) {
        printf("The number is not prime\n");
        return 0;
    }

    
    int is_prime = 1; 
    for (long long i = 2; i * i <= given_number; i++) {
        if (given_number % i == 0) {
            is_prime = 0;
            break;
        }
    }


    if (is_prime) {
        printf("The number is prime\n");
    } else {
        printf("The number is not prime\n");
    }

    return 0;
}
