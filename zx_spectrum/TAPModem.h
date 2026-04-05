#ifndef TAP_MODEM_H
#define TAP_MODEM_H

#include <Arduino.h>

/*
 * ZX Spectrum Tape Timings (in T-states, assuming 3.5MHz clock):
 * One T-state = 1/3,500,000 = 0.2857 microseconds
 *
 * Pilot pulse (half-cycle): 2168 T-states (approx 619 us)
 * First Sync pulse (half-cycle): 667 T-states (approx 191 us)
 * Second Sync pulse (half-cycle): 735 T-states (approx 210 us)
 * Data "0" bit pulse (half-cycle): 855 T-states (approx 244 us)
 * Data "1" bit pulse (half-cycle): 1710 T-states (approx 489 us)
 *
 * Note: Each bit consists of two pulses of the same width.
 */

#define TAP_TSTATE_US (1.0 / 3.5) // Microseconds per T-state

#define TAP_PILOT_T 2168
#define TAP_SYNC1_T 667
#define TAP_SYNC2_T 735
#define TAP_ZERO_T 855
#define TAP_ONE_T 1710

#define TAP_PILOT_HEADER_PULSES 8063
#define TAP_PILOT_DATA_PULSES 3223

class TAPModem {
  public:
    TAPModem(int pin = 9);
    void begin();

    // Low-level pulse generation
    void pulse(uint16_t us);

    // TAP components
    void sendPilot(int pulses, bool startState = LOW);
    void sendSync();
    void sendBit(bool bit);
    void sendByte(byte data);


    // Non-blocking pulse interface
    bool pulseAsync(uint16_t us);
    bool isBufferFull();
    bool isBufferEmpty();

    // Interrupt management
    bool beginInterrupt();
    void endInterrupt();

    // Higher-level TAP block transmission
    // flag_byte is 0x00 for headers, 0xFF for data blocks
    void sendBlock(byte flag_byte, byte* data, int length);

    // Simplified TAP block transmission from a raw buffer (including flag and checksum)
    void sendRawBlock(byte* buffer, int length);

    // Method to generate a tone (reused from SimpleFSKModem)
    void tone(int freq, uint32_t duration_us);

    // New: Support for inverted signals
    void setInverted(bool inverted);

    // New: Turbo speed scaling (1.0 = normal, 2.0 = double speed)
    void setSpeedFactor(float factor);

    // New: Send a string as a Basic header
    void sendBasicHeader(String filename, uint16_t type, uint16_t length, uint16_t param1, uint16_t param2);

    // Standard inter-block pause
    void pause(uint32_t ms = 1000);

    // Internal ISR handler (must be public for ISR to access, or use friend)
    void handleInterrupt();

  private:
    int _pin;
    volatile bool _state;
    bool _inverted;
    volatile uint8_t *_portReg;
    uint8_t _pinMask;
    float _speedFactor;

    uint16_t _pilotUs;
    uint16_t _sync1Us;
    uint16_t _sync2Us;
    uint16_t _zeroUs;
    uint16_t _oneUs;

    // Ring Buffer for pulse widths (Reduced to 64 to save RAM, approx 128 bytes)
    #define PULSE_BUFFER_SIZE 64
    volatile uint16_t _pulseBuffer[PULSE_BUFFER_SIZE];
    volatile uint8_t _head;
    volatile uint8_t _tail;
    volatile bool _running;
};

#endif
