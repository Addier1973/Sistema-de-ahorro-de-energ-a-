#pragma once

// How many ir_values to be stored(I think 250 is enough)
#define IR_MAX_SAMPLES 500
// How many ir_values to be stored(I think 250 is enough)
#define IR_BUFFER_LENGTH (IR_MAX_SAMPLES/2)

#include <stdint.h>

uint16_t get_compressed_value(uint16_t *variants, uint8_t *values, uint16_t index);
