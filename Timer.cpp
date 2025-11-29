#include "Timer.h"
#include "Arduino.h"

extern uint64_t NOW;

// Initialize the timer
Timer::Timer(uint64_t duration) {
    // Sets duration
    this->duration = duration;
    // Sets start time
    this->start = NOW;
    // Sets left_duration to duration
    this->left_duration = duration;
    // If duration is > 0 then "start" the timer
    this->running = duration > 0;
}

// Checks for timeouts(No timeouts if timer is not runnint)
bool Timer::timeout() const {
    if (!this->running)
        return false;

    // Calculate timeout based on left_duration instead of duration
    return NOW >= (this->start + this->left_duration);
}

void Timer::pause() {
    // Running goes to false
    this->running = false;
    // Time already spent since last resume
    uint64_t time_spent = NOW - this->start;
    // Substract it from left_duration
    this->left_duration = this->left_duration - time_spent;
}

// Re-enable the timer
void Timer::resume() {
    this->running = true;
    this->start = NOW;
}

// Reset the timer
void Timer::stop() {
    this->running = false;
    this->left_duration = this->duration;
}

void Timer::restart() {
    this->stop();
    this->resume();
}
