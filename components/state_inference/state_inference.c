#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "freertos/idf_additions.h"

#include "dataset.h"
#include "ap_scan.h"
#include "state_inference.h"
#include "knn.h"

void handle_inference_state(const Dataset *dataset,
                            QueueHandle_t position_queue)
{
    Query query;
    query.aps_count = ap_scan(query.aps);

    Pos pos;
    inference(dataset, &query, &pos);

    xQueueSend(position_queue, (void *)&pos, 0);
}


