#pragma once

#include <stdint.h>

#define MAX_APS 4

typedef struct {
    uint8_t ssid[33];
    uint8_t mac[6];
    int8_t rssi;
    /*int8_t channel;*/
} AccessPoint;

uint16_t wifi_scan(AccessPoint aps[]);
