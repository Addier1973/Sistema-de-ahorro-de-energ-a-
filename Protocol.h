#pragma once

#include "RoomStateMachine.h"
#include "../protocol.pb.h"
#include "CompresedTrain.h"
#include "AC.h"

#define MAX_EVENT_RECORDS 16

struct EventEntry {
    EventKind kind;
    uint32_t args;
    uint32_t timestamp;
};

class Protocol {
    uint8_t slave_id = 1;
    HardwareSerial *port;

    static uint16_t calc_crc(const uint8_t *buff, uint16_t size);

    void send_sync_message();

public:
    Protocol();

    void send_package(uint8_t *payload, uint16_t size);

    void receive_package(uint8_t received_slave_id, uint16_t message_length);

    void process_message(uint8_t slave_id, uint16_t message_length, uint8_t *payload);

    void task();

    uint16_t handle_ir_settings(uint8_t *payload, uint16_t length);

    void handle_settings_command(pb_istream_s *stream);

    bool configured = false;

    SensorStateSnapshot sensors_snapshot;

    bool last_user_ir_already_sent = true;

    void initialize(uint8_t i, AC *pAc);

    AC *ac;

    EventEntry registered_events[MAX_EVENT_RECORDS];
    uint8_t event_count = 0;
};