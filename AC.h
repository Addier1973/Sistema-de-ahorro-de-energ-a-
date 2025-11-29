#pragma once

#include "CompresedTrain.h"
#include "external_configuration.h"

/*
    IR Signal Train Memory/Storage Representation
    ---------------------------------------------
    To save storage/ram I store the signal trains as simple indexes of the real
    values, stored by nibbles (2 for each byte).
    So for the next signal train(in microseconds):

    (original size in bytes for 9 16-bit samples is 18 bytes)
    ... (noise)
    9000
    4500
    4500
    1000
    500
    1000
    500
    500
    1000
    ... (noise)

    I store just the variations sorted decreasingly (9000, 4500, 1000, 500) and then convert the train to indexes, so
    0, 1, 1, 2, 3, 2, 3, 3, 2
    and then store those values as nibbles to save even more space
    (now the size is just 5 bytes plus the train length wich is a single byte, so 6 bytes)
    0x01
    0x12
    0x32
    0x33
    0x20

    I found a simple way to match signal trains and avoid interference with other IR protocols/devices.
    I hardcode the header of the signal train for the AC, so it's matched against the received train,
    and any initial noise is automatically canceled that way, and the header is stripped from the
    captured signal train, saving up some extra bytes, wich is really good.

    The header pattern for the previous example could be 0x12, 0x23, 0x40, with a length of 5 samples(5 nibbles),
    and a padding nibble, summing up 3 bytes (not much, but ... every byte counts).
*/

typedef enum
{
    IREventNone,
    IREventAC,
    IREventOther
} IREventType;

// Pre-declare SmartRoom class
class SmartRoom;

class CompressedIRRecv
{
public:
    void resume();
    void stop();

    bool decode();
};

// AC Handler
class AC
{
public:
    explicit AC();

    void initialize();

    IREventType task();

    void send_compressed_train(uint8_t *data, uint16_t length);

    volatile uint16_t last_user_ir_length = 0;

    // Storage for the IDLE IR Command
    uint16_t ir_idle_length = EXTERNAL_IDLE_IR_LENGTH;
    uint8_t ir_idle_values[IR_MAX_SAMPLES / 2] = {EXTERNAL_IDLE_IR};

    void set_power(bool power_state);

    void resume();
};
