#include "TAPModem.h"

TAPModem::TAPModem(int pin) {
  _pin = pin;
  _state = LOW;
  _inverted = false;
}

void TAPModem::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, _inverted ? HIGH : LOW);
}

void TAPModem::setInverted(bool inverted) {
  _inverted = inverted;
}

void TAPModem::pulse(uint32_t t_states) {
  _state = !_state;
  // Use a local variable to avoid issues with _state toggle
  int val = (_state ^ _inverted) ? HIGH : LOW;
  digitalWrite(_pin, val);

  // Convert T-states to microseconds for delay
  // Using float for precision during delay calculation
  delayMicroseconds((unsigned int)(t_states * TAP_TSTATE_US + 0.5));
}

void TAPModem::sendPilot(int pulses) {
  for (int i = 0; i < pulses; i++) {
    pulse(TAP_PILOT_T);
  }
}

void TAPModem::sendSync() {
  pulse(TAP_SYNC1_T);
  pulse(TAP_SYNC2_T);
}

void TAPModem::sendBit(bool bit) {
  uint32_t t = bit ? TAP_ONE_T : TAP_ZERO_T;
  pulse(t);
  pulse(t);
}

void TAPModem::sendByte(byte data) {
  for (int i = 7; i >= 0; i--) {
    sendBit(bitRead(data, i));
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

  // Pause after block (ZX Spectrum usually pauses 1 second after a block)
  delay(1000);
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
