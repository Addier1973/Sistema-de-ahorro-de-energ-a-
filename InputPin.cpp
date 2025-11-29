#include "InputPin.h"

InputPin::InputPin(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
}

InputPin::~InputPin() = default;

bool InputPin::get() const {
    return !digitalRead(this->pin);
}