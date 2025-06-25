#pragma once

#include "dataset.h"
#include "utils.h"
#include <stdint.h>

#define ALPHA 0.6

typedef struct {
    AccessPoint aps[APS_SIZE];
    uint8_t aps_count;
} Query;

typedef struct {
    double dist;
    Pos pos;
} DistPos;

void handle_inference_state(const Dataset *dataset, Pos *previous,
                            QueueHandle_t position_queue);

void inference(const Dataset *dataset, const Query *query, Pos *result);

double fingerprint_dist(const Fingerprint *fingerprint, const Query *query);
int16_t loss(int8_t rssi_1, int8_t rssi_2);
int cmp(const void *arg_1, const void *arg_2);
