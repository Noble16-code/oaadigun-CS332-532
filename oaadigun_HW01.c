#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// 1. sumOfDigits
int sumOfDigits(int n) {
    if (n <= 0) {
        return -1;
    }
    int sum = 0;
    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }
    return sum;
}

// 2. UABMaxMinDiff
int UABMaxMinDiff(int arr[], int size) {
    if (size <= 0) return 0; // edge case
    int max = arr[0];
    int min = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > max) max = arr[i];
        if (arr[i] < min) min = arr[i];
    }
    return max - min;
}

// 3. replaceEvenWithZero
void replaceEvenWithZero(int arr[], int size, int result[]) {
    for (int i = 0; i < size; i++) {
        if (arr[i] % 2 == 0) {
            result[i] = 0;
        } else {
            result[i] = arr[i];
        }
    }
}

// 4. perfectSquare
int perfectSquare(int n) {
    if (n < 0) return 0; // negatives cannot be perfect squares
    int root = (int) sqrt(n);
    return root * root == n;
}



// 5. countVowels
int countVowels(char s[]) {
    int count = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        char c = tolower(s[i]);
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
            count++;
        }
    }
    return count;
}

// --- MAIN FUNCTION ---

int main() {
    // Test sumOfDigits
    printf("sumOfDigits(123) = %d\n", sumOfDigits(123));   // 6
    printf("sumOfDigits(405) = %d\n", sumOfDigits(405));   // 9
    printf("sumOfDigits(0) = %d\n", sumOfDigits(0));       // -1
    printf("sumOfDigits(7) = %d\n", sumOfDigits(7));       // 7
    printf("sumOfDigits(-308) = %d\n\n", sumOfDigits(-308)); // -1

    // Test UABMaxMinDiff
    int arr1[] = {3, 7, 2, 9};
    int arr2[] = {5, 5, 5, 5, 5, 5};
    int arr3[] = {-2, 4, -1, 6, 5};
    printf("UABMaxMinDiff([3,7,2,9]) = %d\n", UABMaxMinDiff(arr1, 4)); // 7
    printf("UABMaxMinDiff([5,5,5,5,5,5]) = %d\n", UABMaxMinDiff(arr2, 6)); // 0
    printf("UABMaxMinDiff([-2,4,-1,6,5]) = %d\n\n", UABMaxMinDiff(arr3, 5)); // 8

    // Test replaceEvenWithZero
    int arr4[] = {1, 2, 3, 4};
    int arr5[] = {2, 4, 6};
    int arr6[] = {1, 3, 5};
    int res[10];

    replaceEvenWithZero(arr4, 4, res);
    printf("replaceEvenWithZero([1,2,3,4]) = [");
    for (int i = 0; i < 4; i++) printf("%d%s", res[i], (i<3?", ":""));
    printf("]\n");

    replaceEvenWithZero(arr5, 3, res);
    printf("replaceEvenWithZero([2,4,6]) = [");
    for (int i = 0; i < 3; i++) printf("%d%s", res[i], (i<2?", ":""));
    printf("]\n");

    replaceEvenWithZero(arr6, 3, res);
    printf("replaceEvenWithZero([1,3,5]) = [");
    for (int i = 0; i < 3; i++) printf("%d%s", res[i], (i<2?", ":""));
    printf("]\n\n");

    // Test perfectSquare
    printf("perfectSquare(16) = %s\n", perfectSquare(16) ? "True" : "False");
    printf("perfectSquare(15) = %s\n", perfectSquare(15) ? "True" : "False");
    printf("perfectSquare(25) = %s\n", perfectSquare(25) ? "True" : "False");
    printf("perfectSquare(36) = %s\n\n", perfectSquare(36) ? "True" : "False");

    // Test countVowels
    printf("countVowels(\"Hello World\") = %d\n", countVowels("Hello World")); // 3
    printf("countVowels(\"UAB CS\") = %d\n", countVowels("UAB CS"));           // 2
    printf("countVowels(\"Python\") = %d\n", countVowels("Python"));           // 1
    printf("countVowels(\"aeiou\") = %d\n", countVowels("aeiou"));             // 5

    return 0;
}

