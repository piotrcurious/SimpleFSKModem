#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

typedef uint8_t byte;

#define HIGH 0x1
#define LOW  0x0

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

#define bit(b) (1UL << (b))

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define A0 14
#define A1 15

#define BTN_NEXT 7
#define BTN_PREV 6
#define BTN_SEND 3
#define BTN_EJECT 8

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

typedef volatile uint8_t* port_register_t;
uint8_t* portOutputRegister(uint8_t port);
uint8_t digitalPinToPort(uint8_t pin);
uint8_t digitalPinToBitMask(uint8_t pin);

class String : public std::string {
public:
    String(const char* s = "") : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    String(const std::string* s) : std::string(*s) {}
    int length() const { return (int)std::string::length(); }
    bool endsWith(const String& suffix) const {
        if (suffix.length() > length()) return false;
        return std::string::compare(length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    const char* c_str() const { return std::string::c_str(); }
    void trim() {
        size_t first = find_first_not_of(" \t\n\r");
        if (std::string::npos == first) {
            clear();
            return;
        }
        size_t last = find_last_not_of(" \t\n\r");
        *this = substr(first, (last - first + 1));
    }
    void getBytes(byte* buffer, int len) const {
        for (int i = 0; i < len - 1 && i < (int)length(); ++i) {
            buffer[i] = (byte)(*this)[i];
        }
        if (len > 0) {
          buffer[std::min((int)length(), len-1)] = 0;
        }
    }
};

class MockSerial {
public:
    void begin(int baud) {}
    void print(const char* s) { std::cout << s; }
    void print(const String& s) { std::cout << s.c_str(); }
    void print(int n, int base = DEC) {
        if (base == HEX) std::cout << std::hex << n << std::dec;
        else std::cout << n;
    }
    void print(unsigned int n, int base = DEC) {
        if (base == HEX) std::cout << std::hex << n << std::dec;
        else std::cout << n;
    }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(const String& s) { std::cout << s.c_str() << std::endl; }
    void println(int n, int base = DEC) {
        if (base == HEX) std::cout << std::hex << n << std::dec << std::endl;
        else std::cout << n << std::endl;
    }
    void println(unsigned int n, int base = DEC) {
        if (base == HEX) std::cout << std::hex << n << std::dec << std::endl;
        else std::cout << n << std::endl;
    }
    void println(std::string s) { std::cout << s << std::endl; }
};

extern MockSerial Serial;

void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
extern int (*mock_digitalRead)(int pin);
extern int (*mock_analogRead)(int pin);
int digitalRead(int pin);
int analogRead(int pin);
void delayMicroseconds(unsigned int us);
void delay(unsigned long ms);

long map(long x, long in_min, long in_max, long out_min, long out_max);

struct PinEvent {
    int pin;
    int value;
    unsigned long long timestamp_us;
};

extern std::vector<PinEvent> pin_events;
extern unsigned long long current_time_us;

void reset_mock_arduino();

#endif
