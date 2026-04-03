# SimpleFSKModem

Simple FSK modem for Arduino, including specialized support for ZX Spectrum tape emulation.

## Features
- **Standard FSK Modem**: Flexible frequencies and baud rate, default 300 baud (1200/2200 Hz).
- **ZX Spectrum Support**: Specialized `TAPModem` class using cycle-accurate T-state timings for `.tap` files.
- **Microsecond Precision**: High-accuracy signal generation via `delayMicroseconds` and microsecond-level timing constants.
- **SD-Based Senders**: Ready-to-use sketches for playing `.tap` and raw data files from an SD card.

## Library Structure
- `SimpleFSKModem.h/cpp`: General-purpose FSK modem.
- `zx_spectrum/SimpleFSKModem.h/cpp`: ZX Spectrum-compatible FSK modem (1200 baud, 781/1563 Hz).
- `zx_spectrum/TAPModem.h/cpp`: Specialized class for ZX Spectrum `.tap` images using pulse-width modulation.

## Hardware Requirements
- Arduino (e.g., Uno, Nano).
- Audio output pin (default 9).
- SD card module (CS pin 10) for SD sketches.
- Buttons for file selection:
    - BTN_NEXT: Pin 7 (also Pause in playback)
    - BTN_PREV: Pin 6
    - BTN_SEND: Pin 3
    - BTN_EJECT: Pin 8 (Select mode)

## Mock Environment
The project includes a mock Arduino environment in `tests/mock_arduino` for testing project logic on a PC (Linux/macOS/Windows).

To run tests:
```bash
cd tests
g++ -I.. -Izx_spectrum -Imock_arduino test_tap_sd_sketch.cpp ../zx_spectrum/TAPModem.cpp mock_arduino/Arduino.cpp -o test_tap_sd_sketch
./test_tap_sd_sketch
```
