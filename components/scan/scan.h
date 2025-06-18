#pragma once

#include <stdint.h>

#include "freertos/idf_additions.h"
#include "common.h"

#define APS_SIZE 4
#define AP_SCAN_STACK_SIZE 4096

typedef struct {
    uint8_t ssid[33];
    uint8_t mac[6];
    int8_t rssi;
    /*int8_t channel;*/
} AccessPoint;

typedef struct {
    QueueHandle_t queue;
    AccessPoint *total_aps;
    Pos *total_labels;
} ScanParams;

uint16_t ap_scan(AccessPoint aps[]);
TaskHandle_t ap_scan_create(ScanParams *params);
