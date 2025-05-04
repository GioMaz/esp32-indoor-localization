#pragma once

#include <stdint.h>

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

typedef struct {
    int16_t x, y;
} Pos;
