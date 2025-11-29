#include "OutputPin.h"

OutputPin::OutputPin(uint8_t pin, bool inverted)
{
    this->pin = pin;
    this->inverted = inverted;

    pinMode(pin, OUTPUT);
}

OutputPin::~OutputPin() {}

bool OutputPin::get()
{
    return inverted ? !digitalRead(this->pin) : digitalRead(this->pin);
}

void OutputPin::set(bool value)
{
    digitalWrite(this->pin, inverted ? !value : value);
}
