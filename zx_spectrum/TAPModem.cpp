#include "TAPModem.h"


TAPModem* _globalModemPtr = nullptr;

#ifdef ARDUINO_ARCH_AVR
ISR(TIMER1_COMPA_vect) {
    if (_globalModemPtr) _globalModemPtr->handleInterrupt();
}
#endif

TAPModem::TAPModem(int pin) {
  _pin = pin;
  _state = LOW;
  _inverted = false;
  _speedFactor = 1.0f;
  _running = false;
  _head = 0;
  _tail = 0;

  _pilotUs = (uint16_t)(TAP_PILOT_T * TAP_TSTATE_US + 0.5);
  _sync1Us = (uint16_t)(TAP_SYNC1_T * TAP_TSTATE_US + 0.5);
  _sync2Us = (uint16_t)(TAP_SYNC2_T * TAP_TSTATE_US + 0.5);
  _zeroUs  = (uint16_t)(TAP_ZERO_T  * TAP_TSTATE_US + 0.5);
  _oneUs   = (uint16_t)(TAP_ONE_T   * TAP_TSTATE_US + 0.5);
}

void TAPModem::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, _inverted ? HIGH : LOW);
}

void TAPModem::setInverted(bool inverted) {
  _inverted = inverted;
}

void TAPModem::setSpeedFactor(float factor) {
    if (factor < 0.1f) factor = 0.1f;
    if (factor > 5.0f) factor = 5.0f;
    _speedFactor = factor;
}

bool TAPModem::beginInterrupt() {
#ifdef ARDUINO_ARCH_AVR
    _globalModemPtr = this;
    _running = true;

    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    // CTC mode, prescaler 8 -> 2MHz (0.5us per tick)
    OCR1A = 2000; // Initial delay
    TCCR1B = (1 << WGM12) | (1 << CS11); // CTC, prescaler 8
    TIMSK1 |= (1 << OCIE1A);
    sei();
    return true;
#else
    return false;
#endif
}

void TAPModem::endInterrupt() {
#ifdef ARDUINO_ARCH_AVR
    TIMSK1 &= ~(1 << OCIE1A);
    _running = false;
#endif
}

void TAPModem::handleInterrupt() {
#ifdef ARDUINO_ARCH_AVR
    if (_head != _tail) {
        uint16_t us = _pulseBuffer[_head];
        _head = (_head + 1) % PULSE_BUFFER_SIZE;

        _state = !_state;
        digitalWrite(_pin, (_state ^ _inverted) ? HIGH : LOW);

        // OCR1A counts at 2MHz, so 2 ticks per microsecond
        OCR1A = (us * 2) - 1;
    } else {
        // Buffer empty, just wait a bit
        OCR1A = 2000;
    }
#endif
}

bool TAPModem::pulseAsync(uint16_t us) {
    uint8_t next_tail = (_tail + 1) % PULSE_BUFFER_SIZE;
    if (next_tail == _head) return false; // Full

    _pulseBuffer[_tail] = us;
    _tail = next_tail;
    return true;
}

bool TAPModem::isBufferFull() {
    return ((_tail + 1) % PULSE_BUFFER_SIZE) == _head;
}

bool TAPModem::isBufferEmpty() {
    return _head == _tail;
}

void TAPModem::pulse(uint16_t us) {
  // Apply speed scaling: higher factor means shorter pulses
  uint16_t scaledUs = (uint16_t)((float)us / _speedFactor + 0.5f);
  if (scaledUs < 2) scaledUs = 2; // Safety minimum

  if (_running) {
      while (isBufferFull()) {
          // Wait for space in buffer
      }
      pulseAsync(scaledUs);
  } else {
      _state = !_state;
      // Use a local variable to avoid issues with _state toggle
      int val = (_state ^ _inverted) ? HIGH : LOW;
      digitalWrite(_pin, val);
      delayMicroseconds(scaledUs);
  }
}

void TAPModem::sendPilot(int pulses, bool startState) {
  for (int i = 0; i < pulses; i++) {
    pulse(_pilotUs);
  }
}

void TAPModem::sendSync() {
  pulse(_sync1Us);
  pulse(_sync2Us);
}

void TAPModem::sendBit(bool bit) {
  uint16_t us = bit ? _oneUs : _zeroUs;
  pulse(us);
  pulse(us);
}

void TAPModem::sendByte(byte data) {
  for (uint8_t mask = 0x80; mask; mask >>= 1) {
    uint16_t us = (data & mask) ? _oneUs : _zeroUs;
    pulse(us);
    pulse(us);
  }
}

void TAPModem::sendBlock(byte flag_byte, byte* data, int length) {
  sendPilot(flag_byte == 0x00 ? TAP_PILOT_HEADER_PULSES : TAP_PILOT_DATA_PULSES);
  sendSync();

  sendByte(flag_byte);
  byte checksum = flag_byte;
  for (int i = 0; i < length; i++) {
    sendByte(data[i]);
    checksum ^= data[i];
  }
  sendByte(checksum);

  // Pause after block
  delay(1000);
}

void TAPModem::sendRawBlock(byte* buffer, int length) {
  if (length < 1) return;

  // First byte is the flag byte
  byte flag_byte = buffer[0];
  sendPilot(flag_byte == 0x00 ? TAP_PILOT_HEADER_PULSES : TAP_PILOT_DATA_PULSES);
  sendSync();

  for (int i = 0; i < length; i++) {
    sendByte(buffer[i]);
  }

  // Standard inter-block pause
  pause(1000);
}

void TAPModem::pause(uint32_t ms) {
    // End the waveform at the current state
    if (_running) {
        // Approximate number of silent "pulses" to wait
        // since we don't have a silence command, we just wait for buffer to drain
        while (!isBufferEmpty()) {
            // Wait
        }
        delay(ms);
    } else {
        if (ms > 0) delay(ms);
    }
}

// Method to generate a tone of a given frequency and duration on the audio output pin
void TAPModem::tone(int freq, uint32_t duration_us) {
  float period = 1000000.0 / freq;
  float halfPeriod = period / 2.0;
  int cycles = (int)((float)duration_us / period + 0.5);

  for (int i = 0; i < cycles; i++) {
    _state = HIGH;
    digitalWrite(_pin, (_state ^ _inverted) ? HIGH : LOW);
    delayMicroseconds((unsigned int)halfPeriod);
    _state = LOW;
    digitalWrite(_pin, (_state ^ _inverted) ? HIGH : LOW);
    delayMicroseconds((unsigned int)(period - (int)halfPeriod));
  }
}

void TAPModem::sendBasicHeader(String filename, uint16_t type, uint16_t length, uint16_t param1, uint16_t param2) {
  byte header[18];
  header[0] = type & 0xFF;

  // Filename 10 bytes
  for (int i = 0; i < 10; i++) {
    if (i < (int)filename.length()) {
      header[1 + i] = filename[i];
    } else {
      header[1 + i] = ' ';
    }
  }

  header[11] = length & 0xFF;
  header[12] = (length >> 8) & 0xFF;
  header[13] = param1 & 0xFF;
  header[14] = (param1 >> 8) & 0xFF;
  header[15] = param2 & 0xFF;
  header[16] = (param2 >> 8) & 0xFF;

  sendBlock(0x00, header, 17);
}
