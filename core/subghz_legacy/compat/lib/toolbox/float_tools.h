#pragma once

#include <stdbool.h>

static inline bool float_is_equal(float a, float b, float epsilon) {
    float diff = a - b;
    if(diff < 0.0f) diff = -diff;
    return diff <= epsilon;
}
