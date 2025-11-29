#include "CompresedTrain.h"

uint16_t get_compressed_value(uint16_t *variants, uint8_t *values, uint16_t index) {
    if (index & 0b1)
        return variants[values[index / 2] & 0x0F];
    else
        return variants[(values[index / 2] & 0xF0) >> 4];
}
