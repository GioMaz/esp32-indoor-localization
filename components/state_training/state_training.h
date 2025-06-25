#pragma once

#include "dataset.h"
#include "freertos/idf_additions.h"
#include "utils.h"

#define SCAN_ITERATIONS 4

void handle_training_state(Dataset *dataset, Pos *pos, QueueHandle_t position_queue, QueueHandle_t direction_queue, QueueHandle_t scan_queue);
