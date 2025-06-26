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

    // Initialize state and position
    State state = STATE_TRAINING;
    Pos pos = {0, 0};

    // Create direction queue
    QueueHandle_t direction_queue = xQueueCreate(10, sizeof(Pos));

    // Create scan queue
    QueueHandle_t scan_queue = xQueueCreate(10, 1);

    // Create gpio task
    GpioParams gpio_params = {direction_queue, scan_queue, &state, &pos};
    TaskHandle_t gpio_task = gpio_task_create(&gpio_params);

    // Create server task
    ServerWrapper *server = http_server_start(&dataset, &state, &pos);

    while (1) {
        switch (state) {
        case STATE_TRAINING:
            handle_training_state(&dataset, &pos, direction_queue, scan_queue);
            break;
        case STATE_INFERENCE:
            handle_inference_state(&dataset, &pos);
            break;
        }
    }
}
