#include "mock_arduino/Arduino.h"
#include "mock_arduino/SD.h"
#include <iostream>
#include <map>
#include <vector>

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

    // Create a .tap with a long block
    std::string tap_content;
    // Length is 520 (0x0208)
    tap_content.push_back(0x08); // Length LSB
    tap_content.push_back(0x02); // Length MSB
    tap_content.push_back(0xFF); // Flag byte (Data)
    for (int i=0; i<518; i++) tap_content.push_back(i & 0xFF);
    tap_content.push_back(0x00); // Dummy Checksum

    SD.mock_add_file("/test.tap", tap_content);
    SD.mock_slow = true; // 5ms latency

    setup();

    // Play it
    selectMode = false;
    currentTAPFile = "/test.tap";
    sendTAPFile(currentTAPFile);

    std::cout << "Recorded " << pin_events.size() << " pin events." << std::endl;

    unsigned long long last_time = 0;
    int gaps = 0;

    uint32_t zero_pulse = (uint32_t)(TAP_ZERO_T * (1.0/3.5) + 0.5);
    uint32_t one_pulse = (uint32_t)(TAP_ONE_T * (1.0/3.5) + 0.5);

    for (const auto& event : pin_events) {
        if (event.pin == 9) {
            if (last_time > 0) {
                unsigned long long delta = event.timestamp_us - last_time;
                // Allow some tolerance for the pulse widths themselves
                bool is_normal_pulse = (delta >= zero_pulse - 2 && delta <= zero_pulse + 2) ||
                                       (delta >= one_pulse - 2 && delta <= one_pulse + 2) ||
                                       (delta >= (uint32_t)(TAP_PILOT_T * (1.0/3.5) + 0.5) - 2 && delta <= (uint32_t)(TAP_PILOT_T * (1.0/3.5) + 0.5) + 2) ||
                                       (delta >= (uint32_t)(TAP_SYNC1_T * (1.0/3.5) + 0.5) - 2 && delta <= (uint32_t)(TAP_SYNC1_T * (1.0/3.5) + 0.5) + 2) ||
                                       (delta >= (uint32_t)(TAP_SYNC2_T * (1.0/3.5) + 0.5) - 2 && delta <= (uint32_t)(TAP_SYNC2_T * (1.0/3.5) + 0.5) + 2);

                if (!is_normal_pulse && delta < 500000) { // Ignore the 1s inter-block pause
                    std::cout << "TIMING GAP DETECTED: " << delta << " us at " << event.timestamp_us << " us" << std::endl;
                    gaps++;
                }
            }
            last_time = event.timestamp_us;
        }
    }

    std::cout << "Total gaps found: " << gaps << std::endl;

    return 0;
}
