# SimpleFSKModem

Simple FSK modem for Arduino, including specialized support for ZX Spectrum tape emulation.

## Features
- **Standard FSK Modem**: Flexible frequencies and baud rate, default 300 baud (1200/2200 Hz).
- **ZX Spectrum Support**: Specialized `TAPModem` class using cycle-accurate T-state timings for `.tap` files.
- **Interrupt-Driven Waveform Generation**: Uses Timer1 (on AVR) and a 128-entry pulse buffer to ensure gapless audio playback even during SD card reads.
- **Microsecond Precision**: High-accuracy signal generation via microsecond-level timing and direct port manipulation for low-latency ISR.
- **Turbo Support**: Fluent speed scaling (100%–300%) via an analog "Turbo Knob".
- **SD-Based Sender**: Robust `.tap` file streaming from SD card with 512-byte buffering and sector pre-fetching.

## Library Structure
- `SimpleFSKModem.h/cpp`: General-purpose FSK modem.
- `zx_spectrum/TAPModem.h/cpp`: Specialized class for ZX Spectrum `.tap` images using pulse-width modulation.
- `zx_spectrum/SimpleFSKModem.h/cpp`: Spectrum-compatible FSK (1200 baud, 781/1563 Hz).
- `zx_spectrum/send_tap_from_sd.ino`: Serial/Button-only TAP sender.
- `zx_spectrum/send_tap_from_sd_oled.ino`: TAP sender with OLED file browser.

## Hardware Requirements
- Arduino (e.g., Uno, Nano).
- Audio output pin (default 9).
- SD card module (CS pin 10) for SD sketches.
- OLED Display (OLED Version): 128x64 SSD1306 via I2C (A4/SDA, A5/SCL).
- Turbo Knob: Potentiometer on Analog Pin A1.
- Buttons for file selection:
    - BTN_NEXT: Pin 7 (also Pause in playback)
    - BTN_PREV: Pin 6
    - BTN_SEND: Pin 3
    - BTN_EJECT: Pin 8 (Eject/Select mode)

## Mock Environment
The project includes a mock Arduino environment in `tests/mock_arduino` for testing project logic on a PC.

To run a basic timing test:
```bash
g++ -I. -Itests/mock_arduino zx_spectrum/TAPModem.cpp tests/test_tap_modem.cpp tests/mock_arduino/Arduino.cpp -o test_tap_modem
./test_tap_modem
```
