#include "SmartRoom.h"
#include "AC.h"

uint64_t NOW = 0;

SmartRoom::SmartRoom() {

}

void SmartRoom::initialize() {
    // Instantiate IR Device
    this->protocol.initialize(SmartRoomConfiguration::slave_id, &this->ac);

    this->ac.initialize();

    // Create the FSM instance
    this->state_machine.initialize();


    pinMode(SmartRoomConfiguration::rs485_pin, OUTPUT);
    digitalWrite(SmartRoomConfiguration::rs485_pin, LOW);

    pinMode(A0, OUTPUT);
}

[[noreturn]] void SmartRoom::run() {
    while (true) {
        NOW = millis();
        // Heart-beat
        digitalWrite(A0, ((NOW / 500) % 2) == 0);

        // Handle Remote Control commands
        switch (this->ac.task()) {
            case IREventOther:
                this->state_machine.received_ir_message = true;
                break;
            case IREventAC:
                this->state_machine.received_ir_message = true;
                this->protocol.last_user_ir_already_sent = false;
                break;
            default:
                break;
        }

        // Update FSM
        if (this->state_machine.update()) {
            // If there is any change in status update the protocol's snapshot slot
            this->protocol.sensors_snapshot.update(
                    this->state_machine.door_magnetic,
                    this->state_machine.entrance_pir,
                    this->state_machine.room_pir,
                    this->state_machine.ac_power,
                    this->state_machine.guessed_state
            );
        }

        switch (this->state_machine.required_ac_action) {
            case RequiredACAction::PowerOn:
                this->ac.set_power(true);
                break;
            case RequiredACAction::PowerOff:
                this->ac.set_power(false);
                break;
            case RequiredACAction::None:
                break;
        }
        this->state_machine.required_ac_action = RequiredACAction::None;

        // Update stuffs before updating FSM's state, so we can use the differences
        // between the last and current state
        {
            // If the AC power status is ON and ac_poweron_delay finishes turn it ON
            if (this->state_machine.ac_poweron_delay.timeout() && this->state_machine.ac_power) {
                this->ac.set_power(true);
                this->state_machine.ac_poweron_delay.stop();
            }

            // If the AC changed status irSend the IR Command
            if (this->state_machine.ac_power != this->state_machine.last_state.ac_power) {
                if (this->state_machine.ac_power) {
                    this->ac.set_power(true);
                } else {
                    this->ac.set_power(false);
                }

                // If the AC was turned off start the re-start delay timer
                if (!this->state_machine.ac_power) {
                    this->state_machine.ac_poweron_delay.restart();
                }
            }
        }

        // Finally update the FSM status snapshot
        this->state_machine.save_snapshot(&this->state_machine.last_state);

        // Handle communications with Master
        this->protocol.task();
    }
}

bool SmartRoom::register_event(EventKind the_kind, uint32_t the_args) {
    if (this->protocol.event_count >= MAX_EVENT_RECORDS) {
        return false;
    }

    this->protocol.registered_events[this->protocol.event_count++] = (EventEntry) {
            .kind = the_kind,
            .args = the_args,
            .timestamp = millis() / 1000
    };

    return true;
}
