#include "utils.h"
#include <stdio.h>

int toggle_state(State *state)
{
    if (state == NULL) {
        return 1;
    }

    if (*state == STATE_INFERENCE) {
        *state = STATE_TRAINING;
    } else {
        *state = STATE_INFERENCE;
    }

    printf("NEW STATE: %d\n", *state);

    return 0;
}
