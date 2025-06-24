#pragma once

#include "freertos/idf_additions.h"

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
    QueueHandle_t state_queue;
} GpioParams;

void setup_gpio(void);
TaskHandle_t gpio_task_create(GpioParams *gpio_params);
