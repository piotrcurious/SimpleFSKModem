#include "mock_arduino/Arduino.h"
#include <iostream>
#include <vector>
#include "../zx_spectrum/TAPModem.h"

int main() {
    reset_mock_arduino();
    TAPModem modem(9);
    modem.begin();

    // Send a '0' bit: two pulses of 855 T-states
    modem.sendBit(false);

    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;

    for (const auto& event : pin_events) {
        std::cout << "Time: " << event.timestamp_us << " us, Value: " << event.value << std::endl;
    }

    // 855 * 0.2857 = 244.27 us
    // Two pulses should take about 488.5 us

    return 0;
}
