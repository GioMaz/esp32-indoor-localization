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

    memset(dataset, 0, sizeof(*dataset));
    dataset->data_count = 0;

    while (fgets(line, sizeof(line), fp) && dataset->data_count < DATASET_SIZE) {
        int x, y;
        char mac_str[APS_SIZE][13];
        int rssi[APS_SIZE];

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

        Pos pos = { .x = x, .y = y };
        for (int i = 0; i < 4; i++) {
            AccessPoint ap;
            parse_mac(mac_str[i], ap.mac);
            ap.rssi = (int8_t)rssi[i];
            dataset_insert_ap(dataset, &ap, pos);
        }
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

// Find fingerprint at specific position
Fingerprint* find_fingerprint_at_position(Dataset *dataset, int x, int y) {
    for (int i = 0; i < dataset->data_count; i++) {
        if (dataset->data[i].pos.x == x && dataset->data[i].pos.y == y) {
            return &dataset->data[i];
        }
    }
    return NULL;
}

// Get bounds of the dataset
void get_dataset_bounds(Dataset *dataset, int *min_x, int *max_x, int *min_y, int *max_y) {
    if (dataset->data_count == 0) return;
    
    *min_x = *max_x = dataset->data[0].pos.x;
    *min_y = *max_y = dataset->data[0].pos.y;
    
    for (int i = 1; i < dataset->data_count; i++) {
        if (dataset->data[i].pos.x < *min_x) *min_x = dataset->data[i].pos.x;
        if (dataset->data[i].pos.x > *max_x) *max_x = dataset->data[i].pos.x;
        if (dataset->data[i].pos.y < *min_y) *min_y = dataset->data[i].pos.y;
        if (dataset->data[i].pos.y > *max_y) *max_y = dataset->data[i].pos.y;
    }
}

// Static simulation 
void static_simulation(Dataset *dataset) {
    printf("=== STATIC SIMULATION ===\n");
    
    double total_error = 0.0;
    
    // Choose random position to use for the query
    int index = rand() % dataset->data_count;
    Fingerprint *original = &dataset->data[index];
    Pos previous = {0.0, 0.0};
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // New query with noise
        Query query;
        make_noisy_query(original, &query);
        
        // Inference phase
        inference(dataset, &previous, &query);
        
        // Calculate error between real and estimated position
        double dx = previous.x - original->pos.x;
        double dy = previous.y - original->pos.y;
        double error = sqrt(dx * dx + dy * dy);
        total_error += error;
        
        printf("Iteration %3d - Estimated pos: (%.2f, %.2f), Error: %.2f\n", 
               i + 1, previous.x, previous.y, error);
    }
    
    printf("\n------------\n");
    printf("Number of iterations: %d\n", NUM_ITERATIONS);
    printf("Average error: %.2f units\n", total_error / NUM_ITERATIONS);
    printf("\nReal position: (%.2f, %.2f)\n", original->pos.x, original->pos.y);
    printf("Final estimated position: (%.2f, %.2f)\n", previous.x, previous.y);
}

// Dynamic simulation with movement
void dynamic_simulation(Dataset *dataset) {
    printf("\n=== DYNAMIC SIMULATION ===\n");
    
    int min_x, max_x, min_y, max_y;
    get_dataset_bounds(dataset, &min_x, &max_x, &min_y, &max_y);
    
    printf("Dataset bounds: X[%d, %d], Y[%d, %d]\n", min_x, max_x, min_y, max_y);
    
    // Horizontal movement from left to right in the middle row
    int start_x = min_x;
    int end_x = max_x;
    int start_y = min_y + (max_y - min_y) / 2; // Middle row
    int end_y = start_y;
    
    printf("Movement type: Horizontal line (left to right)\n");
    printf("Path: (%d, %d) -> (%d, %d)\n", start_x, start_y, end_x, end_y);
    
    Pos previous = {0.0, 0.0};
    double total_error = 0.0;
    int step_count = 0;
    
    // Simulate horizontal movement from left to right
    for (int current_x = start_x; current_x <= end_x; current_x++) {
        // Find fingerprint at current position
        Fingerprint *current_fp = find_fingerprint_at_position(dataset, current_x, start_y);
        
        if (current_fp != NULL) {
            // Create noisy query for current position
            Query query;
            make_noisy_query(current_fp, &query);
            
            // Perform inference
            inference(dataset, &previous, &query);
            
            // Calculate error
            double dx = previous.x - current_fp->pos.x;
            double dy = previous.y - current_fp->pos.y;
            double error = sqrt(dx * dx + dy * dy);
            total_error += error;
            step_count++;
            
            printf("Step %3d - Real: (%2d, %2d), Estimated: (%.2f, %.2f), Error: %.2f\n",
                   step_count, current_x, start_y, previous.x, previous.y, error);
        } else {
            printf("Step %3d - No fingerprint at position (%d, %d) - Skipping\n",
                   step_count + 1, current_x, start_y);
        }
    }
    
    printf("\n------------\n");
    printf("Dynamic simulation completed\n");
    printf("Total steps with fingerprints: %d\n", step_count);
    if (step_count > 0) {
        printf("Average error: %.2f units\n", total_error / step_count);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <csv_filename>\n", argv[0]);
        return 1;
    }
    
    const char *filename = argv[1];
    
    Dataset dataset;
    if (load_dataset_from_csv(filename, &dataset) != 0) {
        return 1;
    }
    
    printf("Dataset loaded with %u fingerprints\n", dataset.data_count);
    
    srand(time(NULL));
    
    // Run dynamic simulation
    dynamic_simulation(&dataset);
    
    // Run static simulation 
    // static_simulation(&dataset);
    
    return 0;
}