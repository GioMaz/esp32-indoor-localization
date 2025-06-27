#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "dataset.h"
#include "utils.h"
#include "inference.h"

#define NUM_ITERATIONS 10

// Converts MAC string to array of 6 bytes
void parse_mac(const char *mac_str, uint8_t *mac_out) {
    for (int i = 0; i < 6; i++) {
        char byte_str[3] = {mac_str[i * 2], mac_str[i * 2 + 1], '\0'};
        mac_out[i] = (uint8_t)strtol(byte_str, NULL, 16);
    }
}

int load_dataset_from_csv(const char *filename, Dataset *dataset) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error while opening the file");
        return -1;
    }

    char line[256];
    fgets(line, sizeof(line), fp); 

    dataset->data_count = 0;

    while (fgets(line, sizeof(line), fp) && dataset->data_count < DATASET_SIZE) {
        Fingerprint *fp_data = &dataset->data[dataset->data_count];
        fp_data->aps_count = 0;

        int x, y;
        char mac_str[6][13];
        int rssi[6];

        int count = sscanf(line, "%d,%d,%12[^,],%d,%12[^,],%d,%12[^,],%d,%12[^,],%d",
                           &x, &y,
                           mac_str[0], &rssi[0],
                           mac_str[1], &rssi[1],
                           mac_str[2], &rssi[2],
                           mac_str[3], &rssi[3]);

        if (count < 10) {
            fprintf(stderr, "Line format not valid: %s\n", line);
            continue;
        }

        fp_data->pos.x = x;
        fp_data->pos.y = y;

        for (int i = 0; i < 4; i++) {
            parse_mac(mac_str[i], fp_data->aps[i].mac);
            fp_data->aps[i].rssi = (int8_t)rssi[i];
        }

        fp_data->aps_count = 4;
        dataset->data_count++;
    }

    fclose(fp);
    return 0;
}

// Clones a fingerprint into a query and adds noise to RSSI values
void make_noisy_query(const Fingerprint *fp, Query *query) {
    query->aps_count = fp->aps_count;
    for (int i = 0; i < fp->aps_count; i++) {
        memcpy(query->aps[i].mac, fp->aps[i].mac, 6);
        query->aps[i].rssi = fp->aps[i].rssi + (rand() % 11 - 5);
    }
}

int main() {
    Dataset dataset;

    if (load_dataset_from_csv("build/wifi_map.csv", &dataset) != 0) {
        return 1;
    }

    printf("Dataset loaded with %u fingerprints\n", dataset.data_count);

    srand(time(NULL)); 

    double total_error = 0.0;

    // Choose random position to use for the query
    int index = rand() % dataset.data_count;
    Fingerprint *original = &dataset.data[index];

    Pos previous = {0.0, 0.0};  

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        
        // New query with noise
        Query query;
        make_noisy_query(original, &query);

        // Inference phase
        inference(&dataset, &previous, &query);

        // Calculate error between real and estimated position
        double dx = previous.x - original->pos.x;
        double dy = previous.y - original->pos.y;
        double error = sqrt(dx * dx + dy * dy);
        total_error += error;

        printf("Iteration %3d - Estimated pos: (%.2f, %.2f), Error: %.2f\n", i + 1, previous.x, previous.y, error);
    }

    printf("\n------------\n");
    printf("Number of iterations: %d\n", NUM_ITERATIONS);
    printf("Average error: %.2f units\n", total_error / NUM_ITERATIONS);
    printf("\nReal position: (%.2f, %.2f)\n", original->pos.x, original->pos.y);
    printf("Final estimated position: (%.2f, %.2f)\n", previous.x, previous.y);

    return 0;
}
