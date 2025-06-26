#pragma once

#include <stdint.h>

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

typedef struct {
    float x, y;
} Pos;

typedef enum {
    STATE_TRAINING,
    STATE_INFERENCE,
} State;

int toggle_state(State * state);
