
#ifndef SIMPLE_FSK_MODEM_H
#define SIMPLE_FSK_MODEM_H

#include <Arduino.h>

// Define some constants for the FSK parameters
#define FSK_MARK_FREQ 781 // Mark frequency in Hz (ZX-Spectrum compatible)
#define FSK_SPACE_FREQ 1563 // Space frequency in Hz (ZX-Spectrum compatible)
#define FSK_BAUD_RATE 1200 // Baud rate in bits per second (ZX-Spectrum compatible)
#define FSK_BIT_DURATION (1000000 / FSK_BAUD_RATE) // Bit duration in microseconds

// Define some constants for the audio output
#define FSK_AUDIO_PIN 9 // Pin number for audio output
#define FSK_AUDIO_AMP 127 // Amplitude of audio signal (0-255)

// Define some constants for the preamble
#define FSK_PREAMBLE_LENGTH 16 // Number of preamble bits
#define FSK_PREAMBLE_BIT 0x55 // Preamble bit pattern (01010101)

// Define a class for the SimpleFSKModem library
class SimpleFSKModem {
  public:
    // Constructor that takes an optional pin number for audio output
    SimpleFSKModem(int pin = FSK_AUDIO_PIN);

    // Method to initialize the library
    void begin();

    // Method to send a byte of data using FSK modulation
    void sendByte(byte data);

    // Method to send an array of bytes of data using FSK modulation
    void sendBytes(byte* data, int length);

    // Method to send a string of data using FSK modulation
    void sendString(String data);

  private:
    // Variable to store the pin number for audio output
    int _pin;

    // Method to generate a tone of a given frequency and duration on the audio output pin
    void tone(int freq, int duration);
};

#endif
