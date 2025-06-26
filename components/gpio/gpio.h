#pragma once

#include "freertos/idf_additions.h"
#include "utils.h"

#define GPIO_STACK_SIZE 4096

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
} GpioParams;

void setup_gpio(void);
TaskHandle_t gpio_task_create(GpioParams *gpio_params);
