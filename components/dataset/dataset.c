#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "dataset.h"

void dataset_init(Dataset *dataset)
{
    // Reset data_count
    dataset->data_count = 0;

    // Reset each aps_count
    for (int i = 0; i < DATASET_SIZE; i++) {
        dataset->data[i].aps_count = 0;
    }
}

void dataset_insert_ap(Dataset *dataset, AccessPoint *ap, Pos pos)
{
    // Get fingerprint from dataset given the position
    int idx = -1;
    for (int i = 0; i < dataset->data_count; i++) {
        bool check_x = dataset->data[i].pos.x == pos.x;
        bool check_y = dataset->data[i].pos.y == pos.y;
        if (check_x && check_y) {
            idx = i;
        }
    }

    if (idx < 0) {
        idx = dataset->data_count;
        dataset->data_count += 1;
    }

    Fingerprint *fingerprint = &dataset->data[idx];

    // Update rssi if AP is already in fingerprint
    bool found = false;
    for (int i = 0; i < fingerprint->aps_count; i++) {
        if (!memcmp(fingerprint->aps[i].mac, ap->mac, sizeof(Mac))) {
            fingerprint->aps[i].rssi = (fingerprint->aps[i].rssi / 2) + (ap->rssi / 2);
            found = true;
        }
    }

    // Insert new AP if not found
    if (!found && fingerprint->aps_count < APS_SIZE) {
        memcpy(&fingerprint->aps[fingerprint->aps_count], ap, sizeof(*ap));
        fingerprint->aps_count += 1;
        fingerprint->pos = pos;
    }
}

void dataset_print(Dataset *dataset)
{
    for (int i = 0; i < dataset->data_count; i++) {
        Fingerprint *fingerprint = &dataset->data[i];
        printf("FINGERPRINT: (%f, %f)\n",
                fingerprint->pos.x, fingerprint->pos.y);
        for (int j = 0; j < fingerprint->aps_count; j++) {
            printf("\t");
            ap_print(&fingerprint->aps[j]);
        }
        printf("\n");
    }
}

void ap_print(AccessPoint *ap)
{
    printf("RSSI: %d, MAC: %x:%x:%x:%x:%x:%x\n", ap->rssi, ap->mac[0],
           ap->mac[1], ap->mac[2], ap->mac[3], ap->mac[4], ap->mac[5]);
}
