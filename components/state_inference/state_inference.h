#pragma once

#include "dataset.h"
#include "utils.h"
#include "knn.h"
#include <stdint.h>

void handle_inference_state(const Dataset *dataset, QueueHandle_t position_queue);
