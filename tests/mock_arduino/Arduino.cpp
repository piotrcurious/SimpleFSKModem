#include "Arduino.h"
#include "SD.h"

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

int (*mock_digitalRead)(int pin) = nullptr;
int (*mock_analogRead)(int pin) = nullptr;

int digitalRead(int pin) {
    if (mock_digitalRead) return mock_digitalRead(pin);
    return HIGH;
}

int analogRead(int pin) {
    if (mock_analogRead) return mock_analogRead(pin);
    return 0;
}

void delayMicroseconds(unsigned int us) {
    current_time_us += us;
}

void delay(unsigned long ms) {
    current_time_us += (unsigned long long)ms * 1000;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

SDClass SD;
