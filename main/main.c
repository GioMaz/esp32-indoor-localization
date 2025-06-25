#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"

#include "gpio.h"
#include "http_server.h"
#include "setup.h"
#include "state_inference.h"
#include "state_training.h"
#include "storage.h"
#include "utils.h"

void app_main(void)
{
    // Setup peripherals
    setup();

    // Create empty dataset
    Dataset dataset;
    dataset_init(&dataset);

    if (!read_dataset_from_storage(&dataset)) {
        printf("DATASET LOADED\n");
    } else {
        printf("DATASET EMPTY\n");
    }

    // Create direction queue
    QueueHandle_t direction_queue = xQueueCreate(10, sizeof(Pos));

    // Create scan queue
    QueueHandle_t scan_queue = xQueueCreate(10, 1);

    // Create state queue
    QueueHandle_t state_queue = xQueueCreate(10, 1);

    // Create gpio task
    GpioParams gpio_params = {
        direction_queue,
        state_queue,
        scan_queue,
    };
    TaskHandle_t gpio_task = gpio_task_create(&gpio_params);

    // Create server task
    QueueHandle_t position_queue = xQueueCreate(10, sizeof(Pos));
    ServerWrapper *server = http_server_start(position_queue, state_queue,
                                              (const Dataset *)&dataset);

    // Setup training/inference data
    Pos pos_inference = {0, 0};
    Pos pos_training = {0, 0};

    while (1) {
        switch (server->ctx->current_state) {
        case STATE_TRAINING:
            handle_training_state(&dataset, &pos_training, position_queue, direction_queue, scan_queue);
            break;
        case STATE_INFERENCE:
            handle_inference_state(&dataset, &pos_inference, position_queue);
            break;
        }
        unsigned char signal = 0;
        if (xQueueReceive(state_queue, &signal, 0) && signal) { 
            if (server->ctx->current_state == STATE_TRAINING) {
                server->ctx->current_state = STATE_INFERENCE;
            } else {
                server->ctx->current_state = STATE_TRAINING;
            }
            printf("CHANGE STATE to %s\n",
                   server->ctx->current_state == STATE_TRAINING    ? "STATE_TRAINING"
                   : server->ctx->current_state == STATE_INFERENCE ? "STATE_INFERENCE"
                                                                   : "UNKNOWN_STATE");
        }
    }
}
