#ifndef __TOPIC_H
#define __TOPIC_H

#include "arm_math.h"
#include "stdint.h"

typedef struct {
    int16_t counter;
} Encoder;

typedef struct {
    // float32_t angle;
    float angle;
    float velocity;
} StateData;

typedef struct {
    StateData motor;
} State;

#endif /* __TOPIC_H */