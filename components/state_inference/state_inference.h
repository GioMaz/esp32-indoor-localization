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

void handle_inference_state(const Dataset *dataset, QueueHandle_t position_queue);

void inference(const Dataset *dataset, const Query *query, Pos *result);

double fingerprint_dist(Fingerprint *fingerprint, Query *query);
int16_t loss(int8_t rssi_1, int8_t rssi_2);
int cmp(const void *arg_1, const void *arg_2);
