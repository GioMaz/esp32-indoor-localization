#pragma once

#include <stdint.h>

#define MAX_APS 16

typedef struct {
    uint8_t ssid[33];
    uint8_t mac[6];
    int8_t rssi;
    /*int8_t channel;*/
} AccessPoint;

typedef struct {
    double x, y;
} Features;

typedef struct {
    int16_t x, y;
} Label;

typedef struct {
    Features features;
    Label label;
} FeaturesLabel;

void setup_wifi(void);
uint16_t wifi_scan(AccessPoint aps[]);
