#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "freertos/idf_additions.h"

#include "ap_scan.h"
#include "dataset.h"
#include "inference.h"
#include "state_inference.h"

void handle_inference_state(const Dataset *dataset, Pos *previous)
{
    Query query;
    query.aps_count = ap_scan(query.aps);
    inference(dataset, previous, &query);
}
