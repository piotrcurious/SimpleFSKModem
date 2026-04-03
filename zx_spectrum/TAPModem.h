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
    void pulse(uint32_t t_states);

    // TAP components
    void sendPilot(int pulses);
    void sendSync();
    void sendBit(bool bit);
    void sendByte(byte data);

    // Higher-level TAP block transmission
    // flag_byte is 0x00 for headers, 0xFF for data blocks
    void sendBlock(byte flag_byte, byte* data, int length);

    // Simplified TAP block transmission from a raw buffer (including flag and checksum)
    void sendRawBlock(byte* buffer, int length);

    // Method to generate a tone (reused from SimpleFSKModem)
    void tone(int freq, uint32_t duration_us);

  private:
    int _pin;
    bool _state;
};

#endif
