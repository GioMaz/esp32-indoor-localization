#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "freertos/idf_additions.h"

#include "ap_scan.h"
#include "dataset.h"
#include "state_inference.h"

void handle_inference_state(const Dataset *dataset, Pos *previous,
                            QueueHandle_t position_queue)
{
    Query query;
    query.aps_count = ap_scan(query.aps);

    Pos pos;
    inference(dataset, &query, &pos);

    // Calculate moving average
    pos.x *= ALPHA;
    pos.y *= ALPHA;
    pos.x += (1 - ALPHA) * previous->x;
    pos.y += (1 - ALPHA) * previous->y;

    // Update previous
    previous->x = pos.x;
    previous->y = pos.y;

    printf("INFERENCE RESULT: (%f, %f)\n", pos.x, pos.y);
    xQueueSend(position_queue, (void *)&pos, 0);
}

void inference(const Dataset *dataset, const Query *query, Pos *result)
{
    DistPos dps[DATASET_SIZE];

    // Get distances between every fingerprint and every query
    for (int i = 0; i < dataset->data_count; i++) {
        const Fingerprint *fingerprint = &dataset->data[i];
        dps[i].dist = fingerprint_dist(fingerprint, query);
        dps[i].pos = fingerprint->pos;
    }

    // Sort (dist, pos) couples based on dist
    qsort(dps, dataset->data_count, sizeof(dps[0]), cmp);

    for (int i = 0; i < dataset->data_count; i++) {
        printf("POS: (%f, %f), DIST: %f\n", dps[i].pos.x, dps[i].pos.y, dps[i].dist);
    }

    // Take the closest point
    if (dataset->data_count) {
        *result = dps[0].pos;
    } else {
        *result = (Pos){0.0, 0.0};
    }

    printf("ALGO RESULT: (%f, %f)\n", result->x, result->y);
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
    int rssi = 0;
    for (int i = 0; i < aps_count; i++) {
        if (!memcmp(aps[i].mac, mac, sizeof(Mac))) {
            rssi = aps[i].rssi;
        }
    }
    return rssi;
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

int16_t loss(int8_t rssi_1, int8_t rssi_2)
{
    int16_t diff = rssi_1 - rssi_2;
    return abs(diff);
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
