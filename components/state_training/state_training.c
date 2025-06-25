#include "state_training.h"
#include "config.h"
#include "dataset.h"
#include "gpio.h"
#include "ap_scan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"

Pos dir_to_offset[] = {
    [LEFT] = {-1.0, 0.0},
    [RIGHT] = {1.0, 0.0},
    [UP] = {0.0, 1.0},
    [DOWN] = {0.0, -1.0},
};

void handle_training_state(Dataset *dataset, Pos *pos, QueueHandle_t direction_queue, QueueHandle_t scan_queue)
{
    // Check for direction change
    Direction direction;
    if (xQueueReceive(direction_queue, &direction, 0)) {
        pos->x += dir_to_offset[direction].x;
        pos->y += dir_to_offset[direction].y;
    }

    // Check for scan command
    unsigned char signal = 0;
    if (xQueueReceive(scan_queue, &signal, 0) && signal) {
        printf("Scanning position (%f, %f)...\n", pos->x, pos->y);

        for (int i = 0; i < 4; i++) {
            // Block if max datapoints reached
            if (dataset->data_count < DATASET_SIZE) {
                // Create temporary datapoints
                AccessPoint aps[APS_SIZE];

                // Scan datapoints
                uint8_t ap_count = ap_scan(aps);

                // Copy scanned datapoints to dataset
                for (int i = 0; i < ap_count; i++) {
                    dataset_insert_ap(dataset, &aps[i], *pos);
                }
            }

            if (dataset->data_count == DATASET_SIZE) {
                printf("ERROR: Max number of datapoints reached\n");
            }
        }
    }
}
