#include "Arduino.h"

std::vector<PinEvent> pin_events;
unsigned long long current_time_us = 0;

void reset_mock_arduino() {
    pin_events.clear();
    current_time_us = 0;
}

void pinMode(int pin, int mode) {
    // Mock implementation
}

void digitalWrite(int pin, int val) {
    pin_events.push_back({pin, val, current_time_us});
}

void delayMicroseconds(unsigned int us) {
    current_time_us += us;
}

void delay(unsigned long ms) {
    current_time_us += (unsigned long long)ms * 1000;
}
