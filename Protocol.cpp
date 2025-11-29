#include "protocol.pb.h"
#include "pb/pb_decode.h"
#include "pb/pb.h"
#include <EEPROM.h>
#include "Protocol.h"
#include "AC.h"
#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"

#define COMM_BUFFER_LENGTH SERIAL_RX_BUFFER_SIZE

// Some little trick stolen from ModbusSerial.cpp
// Waits while data is still arriving
#define WAIT_FOR_DATA                                              \
    int _len = 0;                                                  \
    while (this->port->available() > _len)                         \
    {                                                              \
        _len = this->port->available();                            \
        delayMicroseconds(SmartRoomConfiguration::rs485_tx_delay); \
    }

//////////////////////////////
// Buffers
//////////////////////////////
extern volatile uint16_t ir_variants[];
extern volatile uint16_t ir_variant_thresholds[];
extern volatile uint8_t ir_pattern_data[];
extern volatile uint8_t ir_values[IR_BUFFER_LENGTH];
//////////////////////////////

//////////////////////////////
// State Machine vars
//////////////////////////////
extern volatile uint8_t ir_variant_count;
extern volatile uint8_t ir_pattern_length;
//////////////////////////////

//////////////////////////////
// Communication
//////////////////////////////
uint8_t comm_buffer[COMM_BUFFER_LENGTH];
uint8_t SmartRoomConfiguration::slave_id = 1;
//////////////////////////////

uint16_t Protocol::calc_crc(const uint8_t *buff, uint16_t size)
{
    uint16_t temp, flag;
    temp = 0xFFFF;

    for (uint8_t i = 0; i < size; i++)
    {
        temp = temp ^ buff[i];
        for (uint8_t j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>= 1;
            if (flag)
                temp ^= 0xA001;
        }
    }

    return temp;
}

void Protocol::send_package(uint8_t *payload, uint16_t size)
{
    uint16_t crc = calc_crc(payload, size);

    digitalWrite(SmartRoomConfiguration::rs485_pin, HIGH);
    delayMicroseconds(SmartRoomConfiguration::rs485_tx_delay);

    // ID
    this->port->write(this->slave_id);

    // Size
    this->port->write((uint8_t)(size & 0x00FF));
    this->port->write((uint8_t)(size >> 8));

    // Payload
    this->port->write(payload, size);

    // CRC
    // Little endian
    this->port->write((uint8_t)(crc & 0x00FF));
    this->port->write((uint8_t)(crc >> 8));

    this->port->flush();
    delayMicroseconds(SmartRoomConfiguration::rs485_post_tx_delay);
    digitalWrite(SmartRoomConfiguration::rs485_pin, LOW);
}

void Protocol::receive_package(uint8_t received_slave_id, uint16_t message_length)
{
    uint16_t crc, calculated_crc;
    bool is_broadcast = received_slave_id == 0;

    if (message_length <= COMM_BUFFER_LENGTH)
    {
        this->port->readBytes(comm_buffer, message_length);
    }
    else
    {
        this->port->readBytes(comm_buffer, COMM_BUFFER_LENGTH);
        return;
    }

    this->port->readBytes((uint8_t *)&crc, 2);

    calculated_crc = calc_crc(comm_buffer, message_length);

    // Send ACK if it's not a broadcast message
    if (!is_broadcast)
    {
        digitalWrite(SmartRoomConfiguration::rs485_pin, HIGH);
        this->port->write(this->slave_id);
        this->port->write((uint8_t)(calculated_crc & 0x00FF));
        this->port->write((uint8_t)(calculated_crc >> 8));
        this->port->flush();
        delayMicroseconds(SmartRoomConfiguration::rs485_post_tx_delay);

        digitalWrite(SmartRoomConfiguration::rs485_pin, LOW);
    }

    if (calculated_crc != crc)
    {
        return;
    }

    this->process_message(received_slave_id, message_length, comm_buffer);
}

void Protocol::task()
{
    if (this->port->available() > 0)
    {
        WAIT_FOR_DATA

        uint8_t package_header[3];
        uint16_t *message_length_ptr = ((uint16_t *)(package_header + 1));

        this->port->readBytes((uint8_t *)&package_header, 3);
        uint8_t received_slave_id = package_header[0];

        if (received_slave_id == this->slave_id)
        {
            this->receive_package(slave_id, (*message_length_ptr));
        }
        else if (received_slave_id == 0)
        {
            this->receive_package(slave_id, (*message_length_ptr));
        }
        else
        {
            size_t bytes_to_skip = this->port->available();

            while (bytes_to_skip > 0)
            {
                bytes_to_skip -= this->port->readBytes((uint8_t *)comm_buffer, min(COMM_BUFFER_LENGTH, bytes_to_skip));
            }
        }
    }
}

void Protocol::process_message(uint8_t slave_id, uint16_t message_length, uint8_t *payload)
{
#define PAYLOAD_KIND_INDEX 0

    auto kind = static_cast<MessageKind>(((uint8_t *)payload)[PAYLOAD_KIND_INDEX]);

    pb_istream_t stream;

    switch (kind)
    {
    case MessageKind_Sync: // debugln(F("Received SYNC Event"))
        this->send_sync_message();
        break;

    case MessageKind_Event: // debugln(F("[Received Event message] This shouldn't happen!"))
        break;

    case MessageKind_IRIdleCommand: // debugln(F("Received IDLE IR"))
#define PAYLOAD_IR_LENGTH_INDEX 1
        this->ac->ir_idle_length = payload[PAYLOAD_IR_LENGTH_INDEX] + (payload[PAYLOAD_IR_LENGTH_INDEX + 1] << 8);
        memcpy(this->ac->ir_idle_values, payload + 3, message_length - 3);
        break;

    case MessageKind_IRCommand: // debugln(F("Received IRCommand"))
        static uint16_t length = (payload[PAYLOAD_IR_LENGTH_INDEX] + (payload[PAYLOAD_IR_LENGTH_INDEX + 1] << 8));
        this->ac->send_compressed_train(payload + 2, length);
        break;

    case MessageKind_Bypass: // debugln(F("Received Bypass Message"))
        break;

    case MessageKind_Settings: // debugln(F("Received Settings Message"))
        static int ir_length = this->handle_ir_settings(payload, message_length - 1);
        stream = pb_istream_from_buffer((const pb_byte_t *)payload + ir_length + 1,
                                        (size_t)message_length - ir_length - 1);

        this->handle_settings_command(&stream);
        break;
    }
}

void Protocol::send_sync_message()
{
    // TODO: Send Alarms and Events

    uint8_t message_count = 0;

    // Skip first byte(later will store the message count)
    uint16_t offset = 1;

    // Ask for configuration and IR Codes
    if (!this->configured)
    {
        message_count++;
        comm_buffer[offset++] = MessageKind_Settings;
    }

    if (!this->last_user_ir_already_sent)
    {
        message_count++;

        // Send the IR Event
        // 1Byte + 1Byte + 2Bytes + (values_len/2)Bytes = 4 + values_len/2
        // Add header to signal the message kind
        comm_buffer[offset++] = MessageKind_IRCommand;
        // Send how many entries were recorded
        comm_buffer[offset++] = (uint8_t)(this->ac->last_user_ir_length & 0x00FF);
        comm_buffer[offset++] = (uint8_t)(this->ac->last_user_ir_length >> 8);

        for (int i = 0; i < ((this->ac->last_user_ir_length + 1) / 2); i++)
        {
            comm_buffer[offset++] = ir_values[i];
        }
    }

    message_count++;
    // TODO: MessageKind_Event should be replaced here
    comm_buffer[offset++] = MessageKind_FSMState;
    comm_buffer[offset++] = (this->sensors_snapshot.door_magnetic ? 1 : 0) +
                            (this->sensors_snapshot.entrance_pir ? 1 << 1 : 0) +
                            (this->sensors_snapshot.room_pir ? 1 << 2 : 0) +
                            (this->sensors_snapshot.ac_power ? 1 << 3 : 0) +
                            (this->sensors_snapshot.guessed_state << 4);

    if (event_count > 0)
    {
        message_count++;
        comm_buffer[offset++] = MessageKind_Event;
        comm_buffer[offset++] = this->event_count;

        for (int i = 0; i < this->event_count; i++)
        {
            EventMessage event_msg = EventMessage_init_default;

            event_msg.kind = this->registered_events[i].kind;
            event_msg.timestamp = this->registered_events[i].timestamp;
            event_msg.value = this->registered_events[i].args;

            size_t count;
            pb_get_encoded_size(&count, &EventMessage_msg, &event_msg);
            comm_buffer[offset++] = (uint8_t)count;
            auto ostream = pb_ostream_from_buffer(comm_buffer + offset, COMM_BUFFER_LENGTH - offset);
            pb_encode(&ostream, EventMessage_fields, &event_msg);
            offset += count;
        }
        event_count = 0;
    }

    comm_buffer[0] = message_count;
    uint16_t payload_length = offset;
    send_package(comm_buffer, payload_length);

    // Clean up flags
    this->last_user_ir_already_sent = true;
}

void Protocol::handle_settings_command(pb_istream_s *stream)
{
    SettingsMessage settings = SettingsMessage_init_default;
    pb_decode(stream, SettingsMessage_fields, &settings);

    ///////////////////
    // SystemSettings
    ///////////////////
#define ASSIGN_IF_NON_ZERO(X)            \
    if (settings.system_settings.X != 0) \
        SmartRoomConfiguration::X = settings.system_settings.X;

    ASSIGN_IF_NON_ZERO(room_empty_timeout)
    ASSIGN_IF_NON_ZERO(room_door_open_timeout)
    ASSIGN_IF_NON_ZERO(power_off_timeout)
    ASSIGN_IF_NON_ZERO(entrance_delay)
    ASSIGN_IF_NON_ZERO(ac_power_on_delay)
    ASSIGN_IF_NON_ZERO(ir_carrier_khz)
    ASSIGN_IF_NON_ZERO(rs485_tx_delay)
    ASSIGN_IF_NON_ZERO(rs485_timeout)
    ///////////////////

    this->configured = true;
}

void Protocol::initialize(uint8_t provided_slave_id, AC *ac_instance)
{
    this->port = &COMMPort;
    this->port->begin(SmartRoomConfiguration::serial_baudrate);
    this->slave_id = provided_slave_id;
    this->ac = ac_instance;

    this->port->setTimeout(1);

    SmartRoomConfiguration::rs485_tx_delay = max(750, 15000000 / SmartRoomConfiguration::serial_baudrate);
    SmartRoomConfiguration::rs485_post_tx_delay = max(1750, 35000000 / SmartRoomConfiguration::serial_baudrate);
}

Protocol::Protocol() {}

uint16_t Protocol::handle_ir_settings(uint8_t *payload, uint16_t length)
{
    // Skip the message kind byte
    payload++;

    ir_variant_count = (*payload++);
    ir_pattern_length = (*payload++);

    for (int i = 0; i < ir_variant_count; i++)
    {
        uint8_t low_part = *payload++;
        uint8_t high_part = *payload++;

        ir_variants[i] = (((uint16_t)high_part) << 8) + low_part;
    }

    for (int i = 0; i < ir_variant_count; i++)
    {
        uint8_t low_part = *payload++;
        uint8_t high_part = *payload++;
        ir_variant_thresholds[i] = (((uint16_t)high_part) << 8) + low_part;
    }

    for (int i = 0; i < ir_pattern_length; i++)
    {
        ir_pattern_data[i] = (*payload++);
    }

    return 2 + (ir_variant_count * 2) + ir_pattern_length;
}
