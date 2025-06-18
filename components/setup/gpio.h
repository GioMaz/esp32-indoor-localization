#pragma once

#include "freertos/idf_additions.h"

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN,
} Direction;

typedef struct {
    QueueHandle_t scan_queue;
} GpioParams;

void setup_gpio(void);
