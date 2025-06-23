#pragma once

#include "dataset.h"
#include "utils.h"
#include <stdint.h>

typedef struct {
    AccessPoint aps[APS_SIZE];
    uint8_t aps_count;
} Query;

void handle_inference_state(const Dataset *dataset, QueueHandle_t position_queue);
