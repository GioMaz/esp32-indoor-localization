#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_log_level.h"
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

static const char *TAG = "main";

void app_main(void)
{
    // Setup peripherals
    setup();

    // Create empty dataset
    Dataset dataset;
    dataset_init(&dataset);

    if (!read_dataset_from_storage(&dataset)) {
        ESP_LOGI(TAG, "DATASET LOADED");
    } else {
        ESP_LOGI(TAG, "DATASET EMPTY");
    }

    State state = STATE_TRAINING;
    bool reset_pos = false;

    // Create direction queue
    QueueHandle_t direction_queue = xQueueCreate(10, sizeof(Pos));

    // Create scan queue
    QueueHandle_t scan_queue = xQueueCreate(10, 1);

    // Create gpio task
    GpioParams gpio_params = {direction_queue, scan_queue, &state, &reset_pos};
    TaskHandle_t gpio_task = gpio_task_create(&gpio_params);

    // Create server task
    QueueHandle_t position_queue = xQueueCreate(10, sizeof(Pos));
    ServerWrapper *server = http_server_start(position_queue, &dataset, &state, &reset_pos);

    // Setup training/inference data
    Pos pos_inference = {0, 0};
    Pos pos_training = {0, 0};

    while (1) {
        switch (state) {
        case STATE_TRAINING:
            handle_training_state(&dataset, &pos_training, position_queue,
                                  direction_queue, scan_queue);
            break;
        case STATE_INFERENCE:
            handle_inference_state(&dataset, &pos_inference, position_queue);
            break;
        }

        // Reset position in case of state change
        if (reset_pos) {
            reset_pos = false;

            ESP_LOGI(TAG, "Reset");

            pos_inference = (Pos){0, 0};
            pos_training = (Pos){0, 0};

            xQueueSend(position_queue, (void *)&pos_inference, 0);
        }
    }
}
