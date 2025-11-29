#pragma once

#include "Timer.h"
#include "SmartRoomConfiguration.h"
#include "InputPin.h"
#include "OutputPin.h"

class SmartRoom;

enum RequiredACAction
{
    None,
    PowerOff,
    PowerOn
};

// Possible states of the room
enum RoomState
{
    // The room is empty
    Empty,
    // The room is possibly occupied, but if there is no activity within a
    // defined timespan the state must go back to Empty
    MaybeOccupied,
    // The room is occupied, this is triggered by the room's PIR sensor and
    // AC/TV remote controllers
    Occupied,
};

// Sensor snapshot struct, replicating the fields of RoomStateMachine
struct SensorStateSnapshot
{
    SensorStateSnapshot();

    void update(bool _door_magnetic, bool _entrance_pir, bool _room_pir, bool _ac_power, RoomState _guessed_state);

    bool door_magnetic = false;
    bool entrance_pir = false;
    bool room_pir = false;
    RoomState guessed_state = Empty;

    bool ac_power;

    uint32_t timestamp;
};

class RoomStateMachine
{
    // Input / Output pins and device definitions
    InputPin door_magnetic_input = InputPin(SmartRoomConfiguration::door_magnetic_pin);
    InputPin entrance_pir_input = InputPin(SmartRoomConfiguration::entrance_pir_pin);
    InputPin room_pir_input = InputPin(SmartRoomConfiguration::room_pir_pin);

    OutputPin tv_power_output = OutputPin(SmartRoomConfiguration::tv_power_pin, true);
    OutputPin room_power_output = OutputPin(SmartRoomConfiguration::room_power_pin, true);

public:
    explicit RoomStateMachine();

    ~RoomStateMachine();

    // Flag to signal IR Commands
    bool received_ir_message = false;
    // Sensor snapshot from last iteration
    SensorStateSnapshot last_state;

    // Iteration callback of the FSM
    // returns true if there is any change in status/sensors
    bool update();

    // Guessed state
    RoomState guessed_state = RoomState::Empty;

    // Update state(and start/stop timers where needed)
    void set_state(RoomState state);

    void save_snapshot(SensorStateSnapshot *snapshot);

    // Keep track of timeouts
    Timer room_empty_timer = Timer(SmartRoomConfiguration::room_empty_timeout);
    Timer room_door_open_timer = Timer(SmartRoomConfiguration::room_door_open_timeout);
    Timer poweroff_timer = Timer(SmartRoomConfiguration::power_off_timeout);
    Timer entrance_timer = Timer(SmartRoomConfiguration::entrance_delay);
    Timer ac_poweron_delay = Timer(SmartRoomConfiguration::ac_power_on_delay);

    RequiredACAction required_ac_action = RequiredACAction::None;

    // Cache for sensor or_values
    bool door_magnetic = false;
    bool entrance_pir = false;
    bool room_pir = false;

    // TV Power Status
    bool tv_power = false;

    bool ac_power = false;

    void initialize();
};
