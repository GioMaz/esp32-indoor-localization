#pragma once

#include "dataset.h"
#include "utils.h"
#include <stdint.h>

typedef struct {
    AccessPoint aps[APS_SIZE];
    uint8_t aps_count;
} Query;

typedef struct {
    double dist;
    Pos pos;
} DistPos;

void handle_inference_state(const Dataset *dataset, const Pos *pos,
                            QueueHandle_t position_queue);

void inference(const Dataset *dataset, const Query *query, Pos *result);
