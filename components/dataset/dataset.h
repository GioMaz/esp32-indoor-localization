#pragma once

#include <stddef.h>
#include <stdint.h>

#include "utils.h"

typedef struct {
    uint8_t mac[6];
    int8_t rssi;
} AccessPoint;

#define APS_SIZE 8

typedef struct {
    AccessPoint aps[APS_SIZE];
    uint8_t aps_count;
    Pos pos;
} Fingerprint;

#define DATASET_SIZE 64

typedef struct {
    Fingerprint data[DATASET_SIZE];
    uint32_t data_count;
} Dataset;

void dataset_init(Dataset *dataset);
void dataset_insert_ap(Dataset *dataset, AccessPoint *ap, Pos pos);
void dataset_print(Dataset *dataset);
void print_ap(AccessPoint *ap);
