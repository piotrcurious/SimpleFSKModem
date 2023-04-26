
#include "SimpleFSKModem.h"

// Constructor that takes an optional pin number for audio output
SimpleFSKModem::SimpleFSKModem(int pin) {
  _pin = pin; // Store the pin number
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
      tone(FSK_MARK_FREQ, FSK_BIT_DURATION);
    } else {
      tone(FSK_SPACE_FREQ, FSK_BIT_DURATION);
    }
  }
}

  // Method to send an array of bytes of data using FSK modulation
  void SimpleFSKModem::sendBytes(byte* data, int length) {
    // Send a preamble of alternating bits before sending the actual data
    for (int i = 0; i < FSK_PREAMBLE_LENGTH; i++) {
      sendByte(FSK_PREAMBLE_BIT);
    }

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
  void SimpleFSKModem::tone(int freq, int duration) {
    // Calculate the period and half-period of the waveform in microseconds
    int period = 1000000 / freq;
    int halfPeriod = period / 2;

    // Calculate the number of cycles to generate for the given duration in microseconds
    int cycles = duration / period;

    // Loop through each cycle and toggle the pin state without any delay between cycles
    for (int i = 0; i < cycles; i++) {
      digitalWrite(_pin, HIGH); // Set the pin high
      delayMicroseconds(halfPeriod); // Wait for half a period
      digitalWrite(_pin, LOW); // Set the pin low
      delayMicroseconds(halfPeriod); // Wait for another half a period
    }
  }
