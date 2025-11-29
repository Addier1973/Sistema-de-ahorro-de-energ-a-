#pragma once

#include <stdint.h>

// Simple pseudo timer implementation
class Timer {
    // Start time(Used to calculate deltas)
    uint64_t start;
    // Timer defined duration
    uint64_t duration;
    // Timer millis left to timeout
    uint64_t left_duration = 0;
    // Is timer running?
    bool running = false;

public:
    explicit Timer(uint64_t duration);

    // Checks if the timer is in timeout state
    bool timeout() const;

    // Pauses the timer
    void pause();

    // Resumes the timer
    void resume();

    // Stops the timer
    void stop();

    // Restarts the timer
    void restart();
};
