#include "mock_arduino/Arduino.h"
#include "mock_arduino/SD.h"
#include <iostream>
#include <map>

// Mock Serial
class MockSerial {
public:
    void begin(int baud) {}
    void print(const char* s) { std::cout << s; }
    void print(String s) { std::cout << s.c_str(); }
    void print(int n) { std::cout << n; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(String s) { std::cout << s.c_str() << std::endl; }
    void println(int n) { std::cout << n << std::endl; }
};
MockSerial Serial;

// Map button pins to states
std::map<int, int> button_states;
int my_digitalRead(int pin) {
    if (button_states.count(pin)) return button_states[pin];
    return HIGH;
}

int my_analogRead(int pin) { return 0; }

#include "../zx_spectrum/TAPModem.h"
#include "../zx_spectrum/send_tap_from_sd.ino"

int main() {
    mock_digitalRead = my_digitalRead;
    mock_analogRead = my_analogRead;
    reset_mock_arduino();

    // Setup mock SD with .tap files
    // Block 1 length 3: [0x00, 0x01, 0x02] (3 bytes total including flag and checksum)
    // Actually .tap block length includes the flag byte and data bytes and checksum.
    // 2-byte length, then 'length' bytes.
    std::string tap_content;
    tap_content.push_back(0x03); // Length LSB
    tap_content.push_back(0x00); // Length MSB
    tap_content.push_back(0x00); // Flag byte (Header)
    tap_content.push_back(0x41); // Data 'A'
    tap_content.push_back(0x41); // Checksum (0x00 ^ 0x41)

    SD.mock_add_file("/game.tap", tap_content);
    SD.mock_add_file("/music.tap", tap_content);

    std::cout << "--- Calling setup() ---" << std::endl;
    setup();

    std::cout << "--- Testing Button NEXT ---" << std::endl;
    button_states[BTN_NEXT] = LOW;
    loop();
    button_states[BTN_NEXT] = HIGH;

    std::cout << "--- Testing Button SEND (Select Mode) ---" << std::endl;
    button_states[BTN_SEND] = LOW;
    loop(); // Sets selectMode = false
    button_states[BTN_SEND] = HIGH;
    for (int i=0; i<10; i++) loop(); // Allow any processing

    std::cout << "--- Sending TAP File (Play Mode) ---" << std::endl;
    button_states[BTN_SEND] = LOW;
    std::cout << "Before loop events: " << pin_events.size() << std::endl;
    loop(); // Triggers sendTAPFile
    std::cout << "After loop events: " << pin_events.size() << std::endl;
    button_states[BTN_SEND] = HIGH;
    for (int i=0; i<10; i++) loop();

    std::cout << "--- Testing Button EJECT ---" << std::endl;
    button_states[BTN_EJECT] = LOW;
    loop(); // Sets selectMode = true
    button_states[BTN_EJECT] = HIGH;

    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;

    return 0;
}
