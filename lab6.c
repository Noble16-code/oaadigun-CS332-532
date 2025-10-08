#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_LISTINGS 1000

// Structure definition
struct listing {
    int id, host_id, minimum_nights, number_of_reviews, calculated_host_listings_count, availability_365;
    char *host_name, *neighbourhood_group, *neighbourhood, *room_type;
    float latitude, longitude, price;
};

// Function to parse one line of CSV into a listing struct
struct listing getfields(char *line) {
    struct listing item;

    item.id = atoi(strtok(line, ","));
    item.host_id = atoi(strtok(NULL, ","));
    item.host_name = strdup(strtok(NULL, ","));
    item.neighbourhood_group = strdup(strtok(NULL, ","));
    item.neighbourhood = strdup(strtok(NULL, ","));
    item.latitude = atof(strtok(NULL, ","));
    item.longitude = atof(strtok(NULL, ","));
    item.room_type = strdup(strtok(NULL, ","));
    item.price = atof(strtok(NULL, ","));
    item.minimum_nights = atoi(strtok(NULL, ","));
    item.number_of_reviews = atoi(strtok(NULL, ","));
    item.calculated_host_listings_count = atoi(strtok(NULL, ","));
    item.availability_365 = atoi(strtok(NULL, ","));

    return item;
}

// Function to print one listing
void displayStruct(struct listing item) {
    printf("%d,%d,%s,%s,%s,%.6f,%.6f,%s,%.2f,%d,%d,%d,%d\n",
        item.id, item.host_id, item.host_name, item.neighbourhood_group,
        item.neighbourhood, item.latitude, item.longitude, item.room_type,
        item.price, item.minimum_nights, item.number_of_reviews,
        item.calculated_host_listings_count, item.availability_365);
}

// Comparison function for sorting by host_name
int compareByHostName(const void *a, const void *b) {
    const struct listing *l1 = (const struct listing *)a;
    const struct listing *l2 = (const struct listing *)b;
    return strcmp(l1->host_name, l2->host_name);
}

// Comparison function for sorting by price
int compareByPrice(const void *a, const void *b) {
    const struct listing *l1 = (const struct listing *)a;
    const struct listing *l2 = (const struct listing *)b;
    if (l1->price < l2->price) return -1;
    else if (l1->price > l2->price) return 1;
    else return 0;
}

// Write sorted data to a new file
void writeToFile(struct listing *list, int count, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%d,%s,%s,%s,%.6f,%.6f,%s,%.2f,%d,%d,%d,%d\n",
            list[i].id, list[i].host_id, list[i].host_name, list[i].neighbourhood_group,
            list[i].neighbourhood, list[i].latitude, list[i].longitude, list[i].room_type,
            list[i].price, list[i].minimum_nights, list[i].number_of_reviews,
            list[i].calculated_host_listings_count, list[i].availability_365);
    }

    fclose(fp);
}

int main() {
    FILE *fptr;
    char line[MAX_LINE];
    struct listing list_items[MAX_LISTINGS];
    int count = 0;

    // Open file
    fptr = fopen("listings.csv", "r");
    if (fptr == NULL) {
        perror("Error opening listings.csv");
        return 1;
    }

    // Skip header line if it exists
    fgets(line, sizeof(line), fptr);

    // Read each line into struct
    while (fgets(line, sizeof(line), fptr) != NULL && count < MAX_LISTINGS) {
        list_items[count++] = getfields(line);
    }
    fclose(fptr);

    printf("Read %d records from listings.csv\n", count);

    // Sort by host_name and write to file
    qsort(list_items, count, sizeof(struct listing), compareByHostName);
    writeToFile(list_items, count, "sorted_by_host.csv");
    printf("Sorted by host_name written to sorted_by_host.csv\n");

    // Sort by price and write to file
    qsort(list_items, count, sizeof(struct listing), compareByPrice);
    writeToFile(list_items, count, "sorted_by_price.csv");
    printf("Sorted by price written to sorted_by_price.csv\n");

    return 0;
}
