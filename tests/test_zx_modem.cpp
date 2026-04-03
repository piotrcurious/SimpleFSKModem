#include "../zx_spectrum/SimpleFSKModem.h"
#include "mock_arduino/Arduino.h"
#include <iostream>

int main() {
    reset_mock_arduino();
    SimpleFSKModem modem(9);
    modem.begin();

    // sendBytes includes preamble
    byte data = 0x01;
    modem.sendBytes(&data, 1);

    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;

    if (pin_events.empty()) {
        std::cerr << "No events recorded!" << std::endl;
        return 1;
    }

    unsigned long long last_time = pin_events[0].timestamp_us;
    for (size_t i = 0; i < std::min((size_t)100, pin_events.size()); ++i) {
        const auto& event = pin_events[i];
        std::cout << event.timestamp_us << "\t" << event.pin << "\t" << event.value << "\t" << (event.timestamp_us - last_time) << std::endl;
        last_time = event.timestamp_us;
    }

    return 0;
}
