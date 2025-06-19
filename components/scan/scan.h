#pragma once

#include <stdint.h>

#include "freertos/idf_additions.h"
#include "common.h"

#define SCAN_SIZE 8
#define AP_SCAN_STACK_SIZE 4096

typedef struct {
    // uint8_t ssid[33];
    uint8_t mac[6];
    int8_t rssi;
    // int8_t channel;
} AccessPoint;

typedef struct {
    QueueHandle_t scan_queue;
    AccessPoint *total_aps;
    Pos         *total_labels;
    uint32_t    *count;
} ScanParams;

uint16_t ap_scan(AccessPoint aps[]);
TaskHandle_t ap_scan_create(ScanParams *params);
