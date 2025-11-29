#include "SmartRoomConfiguration.h"
#include <stdint.h>

uint32_t SmartRoomConfiguration::room_empty_timeout = 500;
uint32_t SmartRoomConfiguration::room_door_open_timeout = 500;
uint32_t SmartRoomConfiguration::power_off_timeout = 500;
uint32_t SmartRoomConfiguration::entrance_delay = 500;
uint32_t SmartRoomConfiguration::ac_power_on_delay = 500;
uint8_t SmartRoomConfiguration::ir_carrier_khz = 38;

uint16_t SmartRoomConfiguration::rs485_tx_delay = 750;
uint16_t SmartRoomConfiguration::rs485_post_tx_delay = 1750;
uint16_t SmartRoomConfiguration::rs485_timeout = 1000;
uint16_t SmartRoomConfiguration::minimum_ac_ir_train_length = 30;
