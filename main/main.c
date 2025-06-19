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
#include "inference.h"
#include "scan.h"
#include "setup.h"
#include "storage.h"
#include "utils.h"

#ifdef CONSOLE
#include "linenoise/linenoise.h"
#define CMD_SIZE 16
#endif

typedef enum {
    STATE_TRAINING,
    STATE_INFERENCE,
} State;

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

    // Create state queue
    QueueHandle_t state_queue = xQueueCreate(10, 1);

    // Create server task
    QueueHandle_t position_queue = xQueueCreate(10, sizeof(Pos));
    ServerWrapper *server = http_server_start(position_queue, state_queue,
                                              (const Dataset *)&dataset);

    // Create scan task
    QueueHandle_t direction_queue = xQueueCreate(10, sizeof(Pos));
    ScanParams scan_params = (ScanParams){
        direction_queue,
        &dataset,
    };
    TaskHandle_t scan = ap_scan_create(&scan_params);

    // Create gpio task
    GpioParams gpio_params = {direction_queue};
    TaskHandle_t gpio_task = gpio_task_create(&gpio_params);

    // Setup training/inference data
    State state = STATE_TRAINING;
    Pos pos = {0, 0};

    while (1) {
        switch (state) {
        case STATE_TRAINING:
            handle_training_state(&dataset, &pos, direction_queue);
            break;
        case STATE_INFERENCE:
            handle_inference_state(&dataset, &pos, position_queue);
            break;
        }
        char signal = 0;
        if (xQueueReceive(state_queue, &signal, 0) && signal) {
            state =
                (state == STATE_TRAINING) ? STATE_INFERENCE : STATE_TRAINING;
            printf("CHANGE STATE\n");
        };
    }
}
