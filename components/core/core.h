#pragma once

#include "utils.h"
#include "dataset.h"
#include <stdint.h>

typedef struct {
    AccessPoint aps[APS_SIZE];
    uint8_t aps_count;
} Query;

void handle_inference_state(Dataset *dataset, Pos *pos, QueueHandle_t position_queue);
void inference(Dataset *dataset, Query *query, Pos *result);
