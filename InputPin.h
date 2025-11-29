#pragma once

#include <Arduino.h>
#include <stdint.h>

class InputPin {
public:
    InputPin(uint8_t pin);

    ~InputPin();

    bool get() const;

private:
    uint8_t pin;
};
