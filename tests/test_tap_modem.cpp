#include "mock_arduino/Arduino.h"
#include <iostream>
#include <vector>
#include "../zx_spectrum/TAPModem.h"

void print_events() {
    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;
    for (const auto& event : pin_events) {
        std::cout << "Time: " << event.timestamp_us << " us, Value: " << event.value << std::endl;
    }
}

int main() {
    std::cout << "--- Standard Speed ---" << std::endl;
    reset_mock_arduino();
    TAPModem modem(9);
    modem.begin();

    // Send a '0' bit: two pulses of 855 T-states (~244 us each)
    modem.sendBit(false);
    print_events();

    std::cout << "\n--- Turbo Speed (2.0x) ---" << std::endl;
    reset_mock_arduino();
    modem.begin(); // Reset state
    modem.setSpeedFactor(2.0f);
    modem.sendBit(false);
    print_events();

    return 0;
}
