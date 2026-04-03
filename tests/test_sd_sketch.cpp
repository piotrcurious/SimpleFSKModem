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

int analog_val = 0;
int my_analogRead(int pin) {
    return analog_val;
}

// Include the sketch code
#include "../zx_spectrum/send_files_from_sd.ino"

int main() {
    mock_digitalRead = my_digitalRead;
    mock_analogRead = my_analogRead;

    reset_mock_arduino();

    // Setup mock SD files
    SD.mock_add_file("list.txt", "file1.tap\nfile2.tap\n");
    SD.mock_add_file("file1.tap", "ABC");
    SD.mock_add_file("file2.tap", "DEF");

    std::cout << "--- Calling setup() ---" << std::endl;
    setup();

    std::cout << "--- Testing Button 6 (Next) ---" << std::endl;
    button_states[SIXTH_BUTTON_PIN] = LOW;
    loop();
    button_states[SIXTH_BUTTON_PIN] = HIGH;

    std::cout << "--- Testing Button 2 (Send) ---" << std::endl;
    button_states[SECOND_BUTTON_PIN] = LOW;
    loop(); // This will trigger handleSecondButton, which sets sendingFile = true
    button_states[SECOND_BUTTON_PIN] = HIGH;

    // The loop should now call sendCurrentFile because sendingFile is true
    loop();

    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;

    return 0;
}
