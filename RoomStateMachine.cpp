#include "RoomStateMachine.h"

RoomStateMachine::RoomStateMachine()
{
}

RoomStateMachine::~RoomStateMachine() = default;

void RoomStateMachine::set_state(RoomState state)
{
    switch (state)
    {
    case Empty: // debugln("Setting state to Empty")
        // Stop timer, because it's already empty
        this->room_empty_timer.stop();
        this->poweroff_timer.restart();
        break;

    case MaybeOccupied: // debugln("Setting state to MaybeOccupied")
        // Restart the timer, keep waiting for more events
        this->room_empty_timer.restart();
        this->poweroff_timer.stop();
        break;

    case Occupied: // debugln("Setting state to Occupied")
        // Stop timer, it's occupied
        this->room_empty_timer.stop();
        this->poweroff_timer.stop();
        break;
    }

    this->guessed_state = state;
}

bool RoomStateMachine::update()
{
    bool something_changed = false;

    entrance_pir = this->entrance_pir_input.get();
    room_pir = this->room_pir_input.get();
    door_magnetic = this->door_magnetic_input.get();

    bool entrance_pir_activity = entrance_pir;
    bool entrance_pir_is_falling = last_state.entrance_pir && !entrance_pir;
    bool room_pir_activity = room_pir;

    bool controller_detected = this->received_ir_message;

    bool room_empty_timeout = this->room_empty_timer.timeout();
    bool room_door_open_timeout = this->room_door_open_timer.timeout();
    bool poweroff_timeout = this->poweroff_timer.timeout();
    bool entrance_delay = this->entrance_timer.timeout();

    bool door_magnetic_is_rising = door_magnetic && !this->last_state.door_magnetic;
    bool door_magnetic_is_falling = !door_magnetic && this->last_state.door_magnetic;

    something_changed = (entrance_pir_activity != last_state.entrance_pir) | (room_pir_activity != last_state.room_pir) | (door_magnetic_is_rising | door_magnetic_is_falling);

    if (door_magnetic_is_falling)
    {
        this->room_door_open_timer.stop();
    }

    if (door_magnetic_is_rising)
    {
        this->room_door_open_timer.restart();
    }

    if (room_door_open_timeout)
    {
        this->room_door_open_timer.restart();
    }

    switch (this->guessed_state)
    {
    case Empty:
        if (entrance_pir_activity)
        {
            something_changed = true;
            this->set_state(RoomState::MaybeOccupied);
        }

        if (door_magnetic_is_falling)
        {
            this->entrance_timer.restart();
        }

        if (poweroff_timeout)
        {
            this->tv_power = false;
            this->required_ac_action = RequiredACAction::PowerOff;
            this->poweroff_timer.stop();
        }

        // Intentional case fallthrough here! (no break statement)
    case MaybeOccupied:
        if (room_pir_activity ||
            controller_detected ||
            // Special case where the Entrance PIR is triggered while the door is closed and entrance delay is on
            (entrance_delay && entrance_pir_activity && !door_magnetic))
        {

            this->set_state(RoomState::Occupied);
            this->tv_power = true;
            this->required_ac_action = RequiredACAction::PowerOn;
            something_changed = true;
        }

        if ((entrance_pir_activity && room_empty_timeout) || entrance_pir_is_falling)
        {
            this->room_empty_timer.restart();
            room_empty_timeout = false;
        }

        // If there is any kind of non-definitive event reset the waiting timer
        if (this->guessed_state == RoomState::Empty &&
            (door_magnetic_is_rising || door_magnetic_is_falling || entrance_pir_activity))
        {
            this->room_empty_timer.restart();
        }

        if (room_empty_timeout)
        {
            this->set_state(RoomState::Empty);
            something_changed = true;
        }

        break;

    case Occupied:
        if (door_magnetic_is_falling)
        {
            this->set_state(RoomState::MaybeOccupied);
            something_changed = true;
        }
        break;
    }

    // Visualize the state with leds
    // digitalWrite(A1, guessed_state == 0);
    // digitalWrite(A2, guessed_state == 1);
    // digitalWrite(A3, guessed_state == 2);

    this->received_ir_message = false;

    // Update TV Power Status
    this->tv_power_output.set(this->tv_power);

    return something_changed;
}

// Create a SensorStateSnapshot from the current FSM state
void RoomStateMachine::save_snapshot(SensorStateSnapshot *snapshot)
{
    if (snapshot == nullptr)
        snapshot = (SensorStateSnapshot *)&this->last_state;

    snapshot->door_magnetic = this->door_magnetic;
    snapshot->entrance_pir = this->entrance_pir;
    snapshot->room_pir = this->room_pir;

    snapshot->ac_power = this->ac_power;
}

void RoomStateMachine::initialize()
{
    this->room_door_open_timer.stop();
    this->room_empty_timer.stop();
    this->entrance_timer.stop();
    this->poweroff_timer.stop();

    this->set_state(RoomState::MaybeOccupied);
}

SensorStateSnapshot::SensorStateSnapshot()
{
    this->door_magnetic = false;
    this->entrance_pir = false;
    this->room_pir = false;
    this->ac_power = false;
    this->timestamp = millis();
}

void SensorStateSnapshot::update(bool _door_magnetic, bool _entrance_pir, bool _room_pir, bool _ac_power,
                                 RoomState _guessed_state)
{
    this->door_magnetic = _door_magnetic;
    this->entrance_pir = _entrance_pir;
    this->room_pir = _room_pir;
    this->ac_power = _ac_power;
    this->timestamp = millis();
    this->guessed_state = _guessed_state;
}