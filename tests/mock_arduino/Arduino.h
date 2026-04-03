#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

typedef uint8_t byte;

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

class String : public std::string {
public:
    String(const char* s = "") : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    int length() const { return std::string::length(); }
    void getBytes(byte* buffer, int len) const {
        for (int i = 0; i < len - 1 && i < (int)length(); ++i) {
            buffer[i] = (byte)(*this)[i];
        }
        if (len > 0) {
          buffer[std::min((int)length(), len-1)] = 0;
        }
    }
};

void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
void delayMicroseconds(unsigned int us);
void delay(unsigned long ms);

struct PinEvent {
    int pin;
    int value;
    unsigned long long timestamp_us;
};

extern std::vector<PinEvent> pin_events;
extern unsigned long long current_time_us;

void reset_mock_arduino();

#endif
