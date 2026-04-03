#include "mock_arduino/Arduino.h"
#include <iostream>
#include <vector>
#include "SimpleFSKModem.h"

int main() {
    reset_mock_arduino();
    SimpleFSKModem modem(9);
    modem.begin();

    std::cout << "--- Default (300 baud) ---" << std::endl;
    modem.sendByte(0x01);
    unsigned long long duration_300 = current_time_us;
    std::cout << "300 baud duration: " << duration_300 << " us" << std::endl;

    reset_mock_arduino();
    std::cout << "--- 1200 baud ---" << std::endl;
    modem.setParameters(1200, 2200, 1200);
    modem.sendByte(0x01);
    unsigned long long duration_1200 = current_time_us;
    std::cout << "1200 baud duration: " << duration_1200 << " us" << std::endl;

    // 300 baud: 1 bit = 3333 us, 8 bits = 26666 us
    // 1200 baud: 1 bit = 833 us, 8 bits = 6666 us

    if (duration_300 > duration_1200 * 3) {
        std::cout << "Baud rate change verified!" << std::endl;
    } else {
        std::cerr << "Baud rate change check failed!" << std::endl;
        return 1;
    }

    return 0;
}
