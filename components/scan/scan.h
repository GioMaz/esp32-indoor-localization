#pragma once

#include <stdint.h>

#include "common.h"
#include "dataset.h"
#include "freertos/idf_additions.h"

#define AP_SCAN_STACK_SIZE 4096

typedef struct {
    QueueHandle_t direction_queue;
    Dataset *dataset;
    uint32_t *dataset_count;
} ScanParams;

uint16_t ap_scan(AccessPoint aps[]);
TaskHandle_t ap_scan_create(ScanParams *params);
