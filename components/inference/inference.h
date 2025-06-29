#pragma once

#include "dataset.h"
#include "utils.h"
#include <stdint.h>

#define ALPHA 0.6

// We assume that the rssi of a non-existent AP is this
#define RSSI_MIN (-100)

typedef struct {
    AccessPoint aps[APS_SIZE];
    uint8_t aps_count;
} Query;

typedef struct {
    double dist;
    Pos pos;
} DistPos;

void inference(const Dataset *dataset, Pos *previous, const Query *query);

double fingerprint_dist(const Fingerprint *fingerprint, const Query *query);
void append_macs(Mac macs[], int *macs_count, const AccessPoint aps[], int aps_count);
int get_rssi(const Mac mac, const AccessPoint aps[], int aps_count);
int cmp(const void *arg_1, const void *arg_2);
