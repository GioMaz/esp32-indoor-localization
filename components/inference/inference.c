#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "dataset.h"
#include "inference.h"

void inference(const Dataset *dataset, Pos *previous, const Query *query)
{
    DistPos dps[DATASET_SIZE];

    // Get distances between every fingerprint and every query
    for (int i = 0; i < dataset->data_count; i++) {
        const Fingerprint *fingerprint = &dataset->data[i];
        dps[i].dist = fingerprint_dist(fingerprint, query);
        dps[i].pos = fingerprint->pos;
    }

#ifdef DEBUG

    // Sort (dist, pos) couples based on dist
    qsort(dps, dataset->data_count, sizeof(dps[0]), cmp);

    for (int i = 0; i < dataset->data_count; i++) {
        printf("POS: (%3.1f, %3.1f), DIST: %3.1f\n", dps[i].pos.x, dps[i].pos.y, dps[i].dist);
    }

    // Take the closest point
    Pos result = {0, 0};
    if (dataset->data_count) {
        result = dps[0].pos;
    }

#else

    // Take the closest point
    int idx = 0;
    for (int i = 1; i < dataset->data_count; i++) {
        if (dps[i].dist < dps[idx].dist) {
            idx = i;
        }
    }

    Pos result = {0, 0};
    if (dataset->data_count) {
        result = dps[idx].pos;
    }

#endif

    printf("KNN RESULT: (%f, %f)\n", result.x, result.y);

    // Calculate moving average
    result.x *= ALPHA;
    result.y *= ALPHA;
    result.x += (1 - ALPHA) * previous->x;
    result.y += (1 - ALPHA) * previous->y;
    previous->x = result.x;
    previous->y = result.y;

    printf("MOVING AVERAGE RESULT: (%f, %f)\n", result.x, result.y);
}

double fingerprint_dist(const Fingerprint *fingerprint, const Query *query)
{
    Mac macs[2 * APS_SIZE];
    int macs_count = 0;

    append_macs(macs, &macs_count, fingerprint->aps, fingerprint->aps_count);
    append_macs(macs, &macs_count, query->aps, query->aps_count);

    int dist = 0;
    for (int i = 0; i < macs_count; i++) {
        int rssi_1 = get_rssi(macs[i], fingerprint->aps, fingerprint->aps_count);
        int rssi_2 = get_rssi(macs[i], query->aps, query->aps_count);
        int dist_i = rssi_1 - rssi_2;
        dist += dist_i * dist_i;
    }

    return dist;
}

void append_macs(Mac macs[], int *macs_count, const AccessPoint aps[], int aps_count)
{
    for (int i = 0; i < aps_count; i++) {
        bool found = false;
        for (int j = 0; j < *macs_count; j++) {
            if (!memcmp(aps[i].mac, macs[j], sizeof(Mac))) {
                found = true;
                break;
            }
        }

        if (!found) {
            memcpy(macs[*macs_count], aps[i].mac, sizeof(Mac));
            *macs_count += 1;
        }
    }
}

int get_rssi(const Mac mac, const AccessPoint aps[], int aps_count)
{
    int rssi = RSSI_MIN;
    for (int i = 0; i < aps_count; i++) {
        if (!memcmp(aps[i].mac, mac, sizeof(Mac))) {
            rssi = aps[i].rssi;
        }
    }
    return rssi;
}

int cmp(const void *arg_1, const void *arg_2)
{
    const double dist_1 = ((const DistPos *)arg_1)->dist;
    const double dist_2 = ((const DistPos *)arg_2)->dist;
    if (dist_1 < dist_2)
        return -1;
    if (dist_1 > dist_2)
        return 1;
    return 0;
}
