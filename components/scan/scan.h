#pragma once

#include <stdint.h>

#define MAX_APS 4
#define AP_SCAN_STACK_SIZE 4096

typedef struct {
    uint8_t ssid[33];
    uint8_t mac[6];
    int8_t rssi;
    /*int8_t channel;*/
} AccessPoint;

uint16_t ap_scan(AccessPoint aps[]);
