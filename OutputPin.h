#pragma once

#include <stdint.h>
#include <Arduino.h>

class OutputPin {
public:
    OutputPin(uint8_t pin,bool inverted=false);

    ~OutputPin();

    bool get();

    void set(bool value);

private:
    uint8_t pin;
    bool inverted=false;
};
