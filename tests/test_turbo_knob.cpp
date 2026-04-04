#include "mock_arduino/Arduino.h"
#include <iostream>
#include <vector>
#include "../zx_spectrum/TAPModem.h"

int main() {
    reset_mock_arduino();
    TAPModem modem(9);
    modem.begin();

    std::cout << "--- Normal Speed (1.0x) ---" << std::endl;
    modem.setSpeedFactor(1.0f);
    modem.sendBit(true); // 1710 T-states = 489 us
    unsigned long long normal_duration = current_time_us;
    std::cout << "Normal pulse: " << normal_duration << " us" << std::endl;

    reset_mock_arduino();
    std::cout << "--- Double Speed (2.0x) ---" << std::endl;
    modem.setSpeedFactor(2.0f);
    modem.sendBit(true); // Should be half duration
    unsigned long long turbo_duration = current_time_us;
    std::cout << "Turbo pulse: " << turbo_duration << " us" << std::endl;

    if (turbo_duration < normal_duration * 0.6) {
        std::cout << "Turbo speed verified!" << std::endl;
    } else {
        std::cerr << "Turbo speed check failed! " << turbo_duration << " vs " << normal_duration << std::endl;
        return 1;
    }

    return 0;
}
