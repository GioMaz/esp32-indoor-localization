#pragma once

#include "network.h"
#include <stdint.h>

typedef struct {
    uint64_t min_mac, max_mac;
    int8_t min_rssi, max_rssi;
} PreprocData;

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
