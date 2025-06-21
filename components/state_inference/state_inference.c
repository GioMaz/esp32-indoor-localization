#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "freertos/idf_additions.h"

#include "dataset.h"
#include "ap_scan.h"
#include "state_inference.h"

void handle_inference_state(const Dataset *dataset,
                            QueueHandle_t position_queue)
{
    Query query;
    query.aps_count = ap_scan(query.aps);

    Pos pos;
    inference(dataset, &query, &pos);

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
        printf("POS: (%d, %d), DIST: %f\n", dps[i].pos.x, dps[i].pos.y, dps[i].dist);
    }

    // Take the mean of the k nearest pos
    *result = (Pos){0, 0};
    int k = sqrt(dataset->data_count);
    if (k > 0) {
        for (int i = 0; i < k; i++) {
            result->x += dps[i].pos.x;
            result->y += dps[i].pos.y;
        }
        result->x /= k;
        result->y /= k;
    }
    printf("INFERENCE RESULT: (%d, %d)\n", result->x, result->y);
}

double fingerprint_dist(const Fingerprint *fingerprint, const Query *query)
{
    double dist = 0;
    int dist_count = 0;

    // Cycle through every AP of the fingerprint
    for (int i = 0; i < fingerprint->aps_count; i++) {
        // Cycle through every AP of the query
        for (int j = 0; j < query->aps_count; j++) {
            const AccessPoint *ap_fingerprint = &fingerprint->aps[i];
            const AccessPoint *ap_query = &query->aps[j];
            if (!memcmp(ap_fingerprint->mac, ap_query->mac, sizeof(ap_query->mac))) {
                dist += loss(ap_fingerprint->rssi, ap_query->rssi);
                dist_count += 1;
            }
        }
    }

    dist /= exp(dist_count);

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
