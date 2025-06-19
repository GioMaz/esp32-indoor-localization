#include <stdbool.h>
#include <string.h>

#include "utils.h"
#include "dataset.h"

void dataset_init(Dataset *dataset)
{
    dataset->data_count = 0;
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

    // Copy AP into fingerprint
    memcpy(&fingerprint->aps[fingerprint->aps_count], ap, sizeof(ap));
    fingerprint->aps_count++;
}
