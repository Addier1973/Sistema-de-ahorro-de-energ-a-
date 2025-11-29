#include <EEPROM.h>
#include "SmartRoom.h"
#include "SmartRoomConfiguration.h"

// Global Smart Room Instance
SmartRoom smart_room = SmartRoom();

void setup() {
    // Configure slave id from EEPROM
    SmartRoomConfiguration::slave_id = EEPROM[0];

    // Communications port (UART0)
    COMMPort.begin(SmartRoomConfiguration::serial_baudrate);

    // Initialize the SmartRoom instance
    smart_room.initialize();

    // Setup pin 13 (Builtin led) as output for debugging
    pinMode(13, OUTPUT);

    // Run it
    smart_room.run();
}

void loop() {}
