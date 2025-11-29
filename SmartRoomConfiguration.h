#pragma once

#include <stdint.h>
#include "Arduino.h"
#include "external_configuration.h"

#define COMMPort Serial

// Configuration for the whole APP
class SmartRoomConfiguration
{
public:
    // Modbus slave id
    static uint8_t slave_id;
    // // SoftwareSerial TX pin
    // static const uint8_t sserial_tx = 4;

    // RS485 Transmit Enable pin
    static const uint8_t rs485_pin = EXTERNAL_RS485_PIN;//= 7;

    // Power relay output pins
    static const uint8_t room_power_pin = EXTERNAL_ROOM_POWER_PIN;//= 6;
    static const uint8_t tv_power_pin = EXTERNAL_TV_POWER_PIN;//= 8;

    // PIR/Magnetic sensor pins
    static const uint8_t room_pir_pin = EXTERNAL_ROOM_PIR_PIN;//= 9;
    static const uint8_t entrance_pir_pin = EXTERNAL_ENTRANCE_PIR_PIN;//= 10;
    static const uint8_t door_magnetic_pin = EXTERNAL_DOOR_MAGNETIC_PIN;//= 11;

    // IR Receiver pin
    static const uint8_t ir_recv_pin = EXTERNAL_IR_RECV_PIN;//= 2;
    // IR Transmitter pin
    static const uint8_t ir_send_pin = EXTERNAL_IR_SEND_PIN;//= 3;

    static uint8_t ir_carrier_khz;

    // Comm baudrate
    static const uint32_t serial_baudrate = EXTERNAL_SERIAL_BAUDRATE;//= 9600;

    // Time to wait before marking the room as empty (in milliseconds)
    static uint32_t room_empty_timeout;
    // Time to wait before alerting that the door is open(in milliseconds)
    static uint32_t room_door_open_timeout;
    // Time to wait before turning off AC and TV(in milliseconds)
    static uint32_t power_off_timeout;
    // Time to wait before reacting to PIR_Entrance after door is closed(in milliseconds)
    static uint32_t entrance_delay;
    // Time to wait before turning the AC on once again(in milliseconds)
    static uint32_t ac_power_on_delay;
    // Time to wait before actually transmitting on RS485 network (in microseconds)
    static uint16_t rs485_tx_delay;
    // Time to wait after transmitting on RS485 network (in microseconds)
    static uint16_t rs485_post_tx_delay;
    // Time to wait for data on RS485 network (in milliseconds)
    static uint16_t rs485_timeout;
    // Minimum length for a signal train to be considered valid AC IR signal train
    static uint16_t minimum_ac_ir_train_length;
};
