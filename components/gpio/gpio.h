#pragma once

#include "freertos/idf_additions.h"
#include "utils.h"
#include <stdbool.h>

#define SCAN_LED GPIO_NUM_2

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN,
} Direction;

typedef struct {
    QueueHandle_t direction_queue;
    QueueHandle_t scan_queue;
    State *state;
    bool *reset_pos;
} GpioParams;

void setup_gpio(void);
TaskHandle_t gpio_task_create(GpioParams *gpio_params);
