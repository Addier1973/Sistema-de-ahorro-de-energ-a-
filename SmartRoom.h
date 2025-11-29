#pragma once

#include <Arduino.h>
#include "RoomStateMachine.h"
#include "SmartRoomConfiguration.h"
#include "InputPin.h"
#include "OutputPin.h"
#include "../protocol.pb.h"
#include "Protocol.h"
#include "AC.h"

// Smart Room definition
class SmartRoom {
public:
    explicit SmartRoom();

    [[noreturn]] void run();

    // The actual FSM instance
    RoomStateMachine state_machine;
    AC ac;

    // The communication layer
    Protocol protocol;

    bool register_event(EventKind kind, uint32_t args = 0);

    void initialize();
};