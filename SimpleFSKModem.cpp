#include "SimpleFSKModem.h"

// Constructor that takes an optional pin number for audio output
SimpleFSKModem::SimpleFSKModem(int pin) {
  _pin = pin; // Store the pin number
  setParameters(DEFAULT_FSK_MARK_FREQ, DEFAULT_FSK_SPACE_FREQ, DEFAULT_FSK_BAUD_RATE);
}

// Method to set FSK parameters
void SimpleFSKModem::setParameters(int markFreq, int spaceFreq, int baudRate) {
  _markFreq = markFreq;
  _spaceFreq = spaceFreq;
  _baudRate = baudRate;
  _bitDurationUs = (1000000UL / baudRate);
}

// Method to initialize the library
void SimpleFSKModem::begin() {
  pinMode(_pin, OUTPUT); // Set the pin mode to output
}

// Method to send a byte of data using FSK modulation
void SimpleFSKModem::sendByte(byte data) {
  // Loop through each bit of the byte, starting from the most significant bit (MSB)
  for (int i = 7; i >= 0; i--) {
    // Get the value of the current bit (0 or 1)
    int bit = bitRead(data, i);

    // Generate a tone of the corresponding frequency (mark or space) for one bit duration
    if (bit == 0) {
      tone(_markFreq, _bitDurationUs);
    } else {
      tone(_spaceFreq, _bitDurationUs);
    }
  }
}

// Method to send an array of bytes of data using FSK modulation
void SimpleFSKModem::sendBytes(byte* data, int length) {
  // Loop through each byte of the array and send it using FSK modulation
  for (int i = 0; i < length; i++) {
    sendByte(data[i]);
  }
}

// Method to send a string of data using FSK modulation
void SimpleFSKModem::sendString(String data) {
  // Convert the string to an array of bytes and send it using FSK modulation
  int length = data.length();
  byte buffer[length];
  data.getBytes(buffer, length + 1);
  sendBytes(buffer, length);
}

// Method to generate a tone of a given frequency and duration on the audio output pin
void SimpleFSKModem::tone(int freq, uint32_t duration_us) {
  // Calculate the period of the waveform in microseconds
  // Use float for better precision during calculation
  float period = 1000000.0 / freq;
  float halfPeriod = period / 2.0;

  // Calculate the number of cycles to generate for the given duration
  int cycles = (int)((float)duration_us / period + 0.5); // Round to nearest cycle

  // Loop through each cycle and toggle the pin state
  for (int i = 0; i < cycles; i++) {
    digitalWrite(_pin, HIGH); // Set the pin high
    delayMicroseconds((unsigned int)halfPeriod); // Wait for half a period
    digitalWrite(_pin, LOW); // Set the pin low
    delayMicroseconds((unsigned int)(period - (int)halfPeriod)); // Wait for the rest of the period to maintain timing
  }
}
