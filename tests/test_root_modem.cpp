#include "../SimpleFSKModem.h"
#include "mock_arduino/Arduino.h"
#include <iostream>
#include <iomanip>

int main() {
    reset_mock_arduino();
    SimpleFSKModem modem(9);
    modem.begin();

    // Send a single bit manually (not possible through the API)
    // Let's send a byte 0x01 (binary 00000001)
    modem.sendByte(0x01);

    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;

    // Analyze the events
    if (pin_events.empty()) {
        std::cerr << "No events recorded!" << std::endl;
        return 1;
    }

    unsigned long long start_time = pin_events[0].timestamp_us;
    unsigned long long last_time = start_time;

    std::cout << "Time (us)\tPin\tValue\tDelta (us)" << std::endl;
    for (const auto& event : pin_events) {
        std::cout << event.timestamp_us << "\t" << event.pin << "\t" << event.value << "\t" << (event.timestamp_us - last_time) << std::endl;
        last_time = event.timestamp_us;
    }

    unsigned long long total_duration = last_time - start_time;
    std::cout << "Total duration: " << total_duration << " us" << std::endl;

    // For 0x01: 7 bits of 0 (MARK) and 1 bit of 1 (SPACE)
    // FSK_BIT_DURATION is 1000/300 = 3 ms = 3000 us (Wait, SimpleFSKModem.h says 1000/300, which is integer 3)
    // In SimpleFSKModem.cpp, tone() uses 1000000/freq for period, and cycles = duration * freq / 1000
    // If duration is 3 ms, and FSK_MARK_FREQ = 1200, cycles = 3 * 1200 / 1000 = 3.6 -> 3 cycles

    return 0;
}
